/*
 * pci_funcs.h
 */
#ifndef _PCI_FUNCS_H
#define _PCI_FUNCS_H

void 	pciinit(void);
int 	pci(int, u8_t, u8_t, u8_t, u32_t, int);

/*** pci.c ***/
pci_link_t *pci_pir_route_interrupt(pcidev_t *);
int 	PCI_route(int,int,...);
void 	show_irqroute(); 
void	show_pirq();
u32_t 	scan_bus(pcibus_t *);
void 	elcr_write_trigger(u32_t,enum intr_trigger);
enum intr_trigger elcr_read_trigger(u32_t);
int 	elcr_probe();
void 	elcr_show();
pcidev_t *find_pcidev(u8_t,u8_t);

/*** pci_name.c ***/
char 	*get_pci_vendor_name(u16_t);
char 	*get_pci_class_name(u8_t);
char 	*get_pci_subclass_name(u8_t, u8_t);
char 	*get_pci_dev(u16_t, u16_t);

/*** pci_pir.c ***/
void	pir_bios(void *);
void 	pci_pir_parse(u32_t);
void 	pci_pir_walk_table(pir_entry_handler *,void *);
void	pci_pir_create_links(PIR_entry_t *,PIR_intpin_t *,void *);
void 	pci_pir_dump_links();
void 	pci_pir_find_link_handler(PIR_entry_t *,PIR_intpin_t *i,void *);
void 	pci_pir_initial_irqs(PIR_entry_t *,PIR_intpin_t *,void *);
void 	pci_print_irqmask(u16_t);
u8_t 	pci_pir_search_irq(u8_t,u8_t,int);
int 	pci_pir_valid_irq(pci_link_t *,int);
int 	pci_pir_choose_irq(pci_link_t *, int);
pci_link_t *pci_pir_find_link(u8_t);
int 	pci_pir_biosroute(u8_t,u8_t,int,int);

/*** pci_bios.c ***/
u64_t 	U64(u32_t, u32_t);
char 	*Str(char *, int);
void 	pir_slot_number(u8_t);
void 	pir_link_bitmap(char, PIR_intpin_t *);
void 	pir_bios(void *);
void 	sm_bios(void *);
void 	bios32(void *);
char 	*pnp_event_notification(u8_t);
void 	pnp_bios(void *);
void 	dmi_bios(void *);
u32_t 	acpiCheckHeader(u32_t *, char *);
void 	acpi_bios(void *);
int 	checksum(u8_t *, size_t);
u32_t 	bios_sigsearch(u32_t, char *, int, int, int );
void 	scan_bios();

void	pci_print_devices(void);

#endif /*_PCI_FUNCS_H*/
