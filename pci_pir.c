/*
 * pci_pir.c
 */
#include <sys/types.h>
#include <sys/seg.h>
#include <sys/errno.h>
#include <sys/immu.h>
#include <sys/param.h>
#include <sys/kmem.h>

#include "pci.h"
#include "pci_bios.h"
#include "pci_funcs.h"

#define NUM_ISA_INTERRUPTS	16

/* IRQS: 3,4,5,6,7,9,10,11,12,14,15 */
#define PCI_IRQ_OVERRIDE_MASK	0xdef8

LIST_HEAD(pci_links);
PIR_table_t *pci_route_table;

int	pci_route_count, pir_bios_irqs=0, pir_parsed;
u32_t	pir_irq_override_mask = PCI_IRQ_OVERRIDE_MASK;
int	pir_interrupt_weight[NUM_ISA_INTERRUPTS];
pcidev_t *pir_device;

void
pci_pir_dump_links(PIR_table_t *pt)
{
	List_t *p;
	pci_link_t *pci_link;
	char	*device_name;

	printf("PCI Route Table\n");

	printf("Router: %02d:%02d.%1d [%4x][%4x]\n",
		pt->pt_header.ph_router_bus,
		PCI_SLOT(pt->pt_header.ph_router_dev_fn),
		PCI_FUNC(pt->pt_header.ph_router_dev_fn),
		pt->pt_header.ph_router_vendor,
		pt->pt_header.ph_router_device);

	device_name = get_pci_dev(pt->pt_header.ph_router_vendor,
		pt->pt_header.ph_router_device);
	if (device_name != 0)
		printf("PCI: IRQ router: %s\n", device_name);

	printf("Link  IRQ  Routed           Allowed IRQs\n");
	list_for_each(p, &pci_links) 
	{
		pci_link=list_entry(p,pci_link_t,list);

		printf("%4x  %3d       %c     ", 
			pci_link->pl_id,
			pci_link->pl_irq, 
			pci_link->pl_routed ? 'Y' : 'N');
		pci_print_irqmask(pci_link->pl_irqmask);
		printf("\n");
	}
}

void
pci_print_irqmask(u16_t code)
{
	if (code == 0)
		printf(" None");
	else
	{
		u8_t 	i;

		for(i=0;i<16;i++)
			if (code & (1 << i))	
				printf(" %d",i);
	}
}

void
pci_pir_parse(u32_t addr)
{
	PIR_table_t *pt;
	int	nroutes;

	DrvDebug(_PCI,5,"pci_pir_parse()\n");

	if (addr == 0)
		return;

	pt = (PIR_table_t *)(u8_t *)(addr);
	if (pt == NULL)
		return;

	if (pt->pt_header.ph_length <= sizeof(PIR_header_t)) 
		return;

	if (bcmp(pt->pt_header.ph_signature,"$PIR",4) != 0)
		return;

	if (!checksum((u8_t *)pt,pt->pt_header.ph_length)) 
		return;

	nroutes = (pt->pt_header.ph_length - 
				sizeof(PIR_header_t)) / sizeof(PIR_entry_t);
	if (nroutes <= 0)
		return;

	pci_route_table = pt;
	pci_route_count = nroutes;

	pci_pir_walk_table(pci_pir_create_links,NULL);
	printf("PCI: PIR table at %x, %d entries\n",addr,pci_route_count);

	if (pci_debug>0)
		pci_pir_dump_links(pt);
}

void
pci_pir_walk_table(pir_entry_handler *handler, void *arg)
{
	PIR_entry_t *entry;
	PIR_intpin_t *intpin;
	int	i, pin;

	DrvDebug(_PCI,5,"pci_pir_walk_table()\n");

	for(i=0; i < pci_route_count; i++) 
	{
		entry = &pci_route_table->pt_entry[i];
		for(pin=0; pin<4; pin++)
		{
			intpin = &entry->pe_intpin[pin];
			/*** Calls pci_pir_create_link() ***/
			if (intpin->link != 0) {
				handler(entry,intpin,arg);
			}
		}
	}
}

void
pci_pir_create_links(PIR_entry_t *entry, PIR_intpin_t *intpin, void *arg)
{
	pci_link_t *pci_link;

	DrvDebug(_PCI,9,"pci_pir_create_links()\n");

	pci_link = pci_pir_find_link(intpin->link);
	if (!pci_link) {
		DrvDebug(_PCI,5,"pci_pir_create_links: new link 0x%02x mask 0x%04x\n", 
			intpin->link, intpin->irqs);

		pci_link=kmem_zalloc(sizeof(pci_link_t), KM_NOSLEEP);
		if (pci_link == NULL) {
			printf("$PIR: out of memory for link 0x%02x\n",intpin->link);
			return;
		}
		pci_link->pl_id = intpin->link;
		pci_link->pl_irqmask = intpin->irqs;
		pci_link->pl_irq = PCI_INVALID_IRQ;
		pci_link->pl_references = 1;
		pci_link->pl_routed = 0;
		list_add_tail(&pci_link->list, &pci_links);
	} else {
		pci_link->pl_references++;
		if (intpin->irqs != pci_link->pl_irqmask) {
			printf("Whoops: Different mask for link, merging\n");
			pci_link->pl_irqmask &= intpin->irqs;
		}
	}
}

pci_link_t *
pci_pir_find_link(u8_t link_id)
{
	pci_link_t *pci_link;
	List_t	*p;

	list_for_each(p, &pci_links) 
	{
		pci_link=list_entry(p,pci_link_t,list);

		if (pci_link->pl_id == link_id) {
			return (pci_link);
		}
	}
	return NULL;
}

void
pci_pir_find_link_handler(PIR_entry_t *ent, PIR_intpin_t *intpin, void *arg)
{
	pci_link_lookup_t *lkup;
	int pin=(intpin - ent->pe_intpin);

	lkup = (pci_link_lookup_t *)arg;

	if (ent->pe_bus == lkup->bn &&
	    ent->pe_devfn == lkup->devfn &&
	    pin == lkup->pin) {
		*lkup->pci_link_ptr = pci_pir_find_link(intpin->link);
	}
}

int
pci_pir_valid_irq(pci_link_t *pci_link, int irq)
{
	if (pci_link == NULL) 
		return 0;

	if (!PCI_INTERRUPT_VALID(irq)) 
		return 0;

	return (pci_link->pl_irqmask & (1 << irq));
}

u8_t 
pci_pir_search_irq(u8_t bn, u8_t devfn, int pin)
{
	u8_t	intpin, intline;
	u16_t	vid;

	if ((prcw(bn,devfn,PCI_VENDOR_ID,&vid) != 0) || 
	    (vid == PCIV_INVALID)) return PCI_INVALID_IRQ;

	if ((prcb(bn,devfn,PCI_INTERRUPT_PIN,&intpin) != 0) || 
	    (intpin != pin+1)) return PCI_INVALID_IRQ;

	if ((prcb(bn,devfn,PCI_INTERRUPT_LINE,&intline) != 0) || 
	    (intline == PCI_INVALID_IRQ)) return PCI_INVALID_IRQ;

	return intline;
}

void
pci_pir_initial_irqs(PIR_entry_t *entry, PIR_intpin_t *intpin, void *arg)
{
	pci_link_t *pci_link;
	u8_t 	irq, pin;

	pin = intpin - entry->pe_intpin;

	irq = pci_pir_search_irq(entry->pe_bus,entry->pe_devfn,pin);
	if (irq == PCI_INVALID_IRQ) return;

	pci_link = pci_pir_find_link(intpin->link);
	if (pci_link == NULL) return;
	if (irq == pci_link->pl_irq) return;

	if (irq >= NUM_ISA_INTERRUPTS) {
		printf("Ignoring invalid BIOS IRQ %d from %d.%d.INT#%c for link %x\n",
			irq, entry->pe_bus, entry->pe_devfn, 
			IntPin(pin), pci_link->pl_id);
		return;
	}

	if (pci_link->pl_irq == PCI_INVALID_IRQ) {
		if (!pci_pir_valid_irq(pci_link,irq))
			printf("Using invalid BIOS IRQ %d from %d.%d.INT#%c for link %x\n",
				irq, entry->pe_bus, entry->pe_devfn, 
				IntPin(pin), pci_link->pl_id);
		pci_link->pl_irq = irq;
		pci_link->pl_routed = 1;
		return;
	}

	if (!pci_pir_valid_irq(pci_link, irq)) {
		printf("$PIR: BIOS IRQ is not valid\n");
	} else
	if (!pci_pir_valid_irq(pci_link, pci_link->pl_irq)) {
		printf("$PIR: Preferring valid BIOS IRQ\n");
		pci_link->pl_irq = irq;
		pci_link->pl_routed = 1;
	} else 
		printf("$PIR: BIOS IRQ does not match link\n");
}

int
pci_pir_choose_irq(pci_link_t *pci_link, int irqmask)
{
	int	i, irq, realmask;

	realmask = pci_link->pl_irqmask & irqmask;
	if (realmask == 0) 
		return PCI_INVALID_IRQ;

	irq=PCI_INVALID_IRQ;
	for(i=0; i<NUM_ISA_INTERRUPTS; i++) {
		if (realmask & (1 << i)) {
			irq=i;
			break;
		}
	}
	if (irq == PCI_INVALID_IRQ)
		return PCI_INVALID_IRQ;

	for(i=irq+1; i<NUM_ISA_INTERRUPTS; i++) {
		if (!(realmask & (1<<i))) 
			continue;
		if (pir_interrupt_weight[i] < pir_interrupt_weight[irq])
			irq=i;
	}
	printf("$PIR: Found IRQ %d for link %x from ",irq,pci_link->pl_id);
	pci_print_irqmask(realmask);	
	printf("\n");
	return irq;
}

pci_link_t *
pci_pir_route_interrupt(pcidev_t *pdev)
{
	pci_link_lookup_t lookup;
	pci_link_t *pci_link=NULL;
	int	irq, error;

	DrvDebug(_PCI,5,"pci_pir_route_interrupt()\n");

	if (pdev == NULL)
		return NULL;
	
	if (pdev->intpin < 1 || pdev->intpin > 4)
		return NULL;

	if (pci_route_table == NULL) 
		return NULL;

	lookup.bn = pdev->bn;
	lookup.devfn = pdev->devfn;
	lookup.pin = pdev->intpin-1;
	lookup.pci_link_ptr = &pci_link;

	pci_pir_walk_table(pci_pir_find_link_handler,&lookup);
	if (pci_link == NULL) {
		printf("$PIR: No matching entry for %02d.%02d.%d INT#%c\n",
			pdev->bn,
			PCI_SLOT(pdev->devfn),	
			PCI_FUNC(pdev->devfn),
			IntPin(pdev->intpin));
		return NULL;
	}

	if (!PCI_INTERRUPT_VALID(pci_link->pl_irq)) {
		if (pci_link->pl_irqmask != 0 && powerof2(pci_link->pl_irqmask))
			irq=ffs(pci_link->pl_irqmask)-1;
		else
			irq=pci_pir_choose_irq(pci_link,
				pci_route_table->pt_header.ph_pci_irqs);

		if (!PCI_INTERRUPT_VALID(irq))
			irq=pci_pir_choose_irq(pci_link,pir_bios_irqs);

		if (!PCI_INTERRUPT_VALID(irq))
			irq=pci_pir_choose_irq(pci_link,pir_irq_override_mask);

		if (!PCI_INTERRUPT_VALID(irq))
			return NULL;

		pci_link->pl_irq=irq;
	}

	if (!pci_link->pl_routed) {
		error = pci_pir_biosroute(pdev->bn,
					  pdev->devfn,
					  lookup.pin, 
					  irq);
		if (error != 0)
			return NULL;
		
		error = BUS_CONFIG_INTR(pir_device,
			    		irq,
			    		INTR_TRIGGER_LEVEL, 
			    		INTR_POLARITY_LOW);
		if (error != 0)
			return NULL;

		pci_link->pl_routed = 1;
	}
#if 0 
	printf("$PIR: %02d:%02d.%d INT#%c routed to irq %d\n",
		pdev->bn,
		PCI_SLOT(pdev->devfn),
		PCI_FUNC(pdev->devfn),
		IntPin(pdev->intpin), 
		pci_link->pl_irq);
#endif
	return pci_link;
}

/*
 * freebsd atpic.c
 */
int
BUS_CONFIG_INTR(pcidev_t *pcidev,int irq,enum intr_trigger trig,enum intr_polarity pol)
{
	u32_t	vector;
	extern int elcr_found;
	extern	int	level_intr_mask;	/*KERNEL*/

	DrvDebug(_PCI,5,"BUS_CONFIG_INTR()\n");

	if (trig == INTR_TRIGGER_CONFORM)
		trig = INTR_TRIGGER_EDGE;
	if (pol == INTR_POLARITY_CONFORM)
		pol = INTR_POLARITY_HIGH;

	vector=irq;
	/* vector=atpic_vector(isrc);*/

	if ((trig == INTR_TRIGGER_EDGE  && pol == INTR_POLARITY_LOW) ||
	    (trig == INTR_TRIGGER_LEVEL && pol == INTR_POLARITY_HIGH)) {
		printf("PIC: Mismatch config for IRQ trigger\n");
		return EINVAL;
	}

	if ((vector == 0 || vector == 1 || vector == 2 || vector == 13) &&
	    trig == INTR_TRIGGER_LEVEL) {
		printf("PIC: Ignoring invalid level/low config for IRQ\n");
		return EINVAL;
	}

	if (!elcr_found) {
		printf("PIC: No ELCR to configure IRQ\n");
		return ENXIO;
	}

	level_intr_mask |= (1<<irq);
	elcr_write_trigger(irq,trig);
	return 0;
}

int
pci_pir_biosroute(u8_t bus,u8_t devfn,int pin,int irq)
{
	DrvDebug(_PCI,5,"pci_pir_biosroute()\n");

	if (!pci_router || !pci_router->route) {
		printf("$PIR: biosroute: no router installed\n");
		return ENXIO;
	}
	
	if (!pir_router_found) {
		printf("$PIR: biosroute: router location unknown\n");
		return ENXIO;
	}
	return pci_router->route(PIRQ_SET, pin+1, irq);
}
