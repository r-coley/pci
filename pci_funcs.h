/*
 * pci_funcs.h
 */
#ifndef _PCI_FUNCS_H
#define _PCI_FUNCS_H


/*** pci.c ***/
void 	pciinit();
int 	pci(int, u8_t, u8_t, u8_t, u32_t, int);
pci_link_t *pci_pir_route_interrupt(pcidev_t *);
pcidev_t *find_pcidev(u8_t,u8_t);
u32_t 	scan_bus(pcibus_t *);
int 	RegisterIRQ(char *,int,int (*func)(),int,enum intr_trigger);
int	request_mem_region(u32_t, u32_t, char *name);
int	release_mem_region(u32_t, u32_t);
int	request_io_region(u32_t, u32_t, char *name);
int	release_io_region(u32_t, u32_t);
void 	pci_format_one_bar(resource_t *r, int barno, char *buf, int maxlen, int *len, int indent);
void 	reverse_bus_devices(pcibus_t *);
pcidev_t *reverse_dev_list(pcidev_t *);

/*** pci_name.c ***/
char 	*get_pci_vendor_name(u16_t);
char 	*get_pci_class_name(u8_t);
char 	*get_pci_subclass_name(u8_t, u8_t);
char 	*get_pci_dev(u16_t, u16_t);

/*** pci_pir.c ***/
void 	pci_pir_parse(u32_t);
void 	pci_pir_walk_table(pir_entry_handler *,void *);
void	pci_pir_create_links(PIR_entry_t *,PIR_intpin_t *,void *);
void 	pci_pir_dump_links(PIR_table_t *);
void 	pci_pir_find_link_handler(PIR_entry_t *,PIR_intpin_t *i,void *);
void 	pci_pir_initial_irqs(PIR_entry_t *,PIR_intpin_t *,void *);
void 	pci_print_irqmask(u16_t);
u8_t 	pci_pir_search_irq(u8_t,u8_t,int);
int 	pci_pir_valid_irq(pci_link_t *,int);
int 	pci_pir_choose_irq(pci_link_t *, int);
pci_link_t *pci_pir_find_link(u8_t);
int 	pci_pir_biosroute(u8_t,u8_t,int,int);
void 	pci_pir_init_weights(void);

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
void 	scan_bios(void);

void	pci_print_devices(void);
void 	pci_print_one_device(pcidev_t *dev);

/*** pci_chipsets.c */
int 	I440FX_init(void);
int 	I440FX_info(void);
int 	I440FX_probe_pci(pcidev_t *, pcidevid_t *);
int 	I440FX_route(int cmd,int,...);
int 	PIIX4_init(void);
int 	PIIX4_info(void);
int 	PIIX4_probe_pci(pcidev_t *, pcidevid_t *);
int 	PIIX4_route(int,int,...);
int 	PIIX3_init(void);
int 	PIIX3_info(void);
int 	PIIX3_probe_pci(pcidev_t *, pcidevid_t *);
int 	PIIX3_route(int,int,...);
int 	VIA_init(void);
int 	VIA_info(void);
int 	VIA_probe_pci(pcidev_t *, pcidevid_t *);
int 	VIA_route(int,int,...);
int 	pci_slot_get_pirq(u8_t, int);
int 	pci_slot_get_apic_pirq(u8_t, int);
int 	PCI_get_interrupt(pcidev_t *);
void 	show_irqroute();
void 	show_pirq();
void 	elcr_show();
int 	elcr_probe();
enum 	intr_trigger elcr_read_trigger(int);
void 	elcr_write_trigger(int, enum intr_trigger);
void 	myset_elt(int, int);
int 	pci_probe_chipsets();
int	PCI_route(int, int, ...);
int	pci_device_enable(pcidev_t *dev);
int	pci_register_driver(struct pci_driver *drv);
void	pci_set_master(pcidev_t *dev);

#endif /*_PCI_FUNCS_H*/
