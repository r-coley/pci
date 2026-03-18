/*
 * pci_chipsets.c
 */
#include <sys/types.h>
#include <stdarg.h>
#include <sys/errno.h>
#include <sys/eisa.h>
#include <sys/ipl.h>
#include <sys/pic.h>

#include "pci.h"
#include "pci_ids.h"
#include "pci_funcs.h"

extern	int	splvalid_flag;

extern int pci_debug;

int	elcr_status;
int	elcr_found=0;

i440fx_t	i440;
piix3_t		piix3;
piix4_t		piix4;
via_t		via;

pci_router_ops_t pci_generic_router_ops =
{
	"PCI",
	PCI_route,
	0x60
};

int I440FX_init();
static pcidevid_t I440FX_pci_tbl[] = 
{
    { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82441) },
    { 0, }
};

struct pci_driver I440FX_driver = 
{
	"I440FX",
	I440FX_probe_pci,
	0,
	I440FX_pci_tbl,
	0
};

static pci_router_ops_t i440fx_router_ops =
{
	"I440FX",
	I440FX_route,
	0x60
};

int 
I440FX_init()
{
	int	I440FX_have_pci=0;

	if (pci_devices == 0)
		pciinit();

	if (pci_register_driver(&I440FX_driver))
		I440FX_have_pci=1;
	else
		DrvDebug(_PCI,5,"I440FX_init(): Not Found\n");

	return (I440FX_have_pci) ? 0 : -ENODEV;
}

int
I440FX_probe_pci(pcidev_t *pdev, pcidevid_t *ent)
{
	int	err=0, i, pin;

	DrvDebug(_PCI,5,"I440FX_probe_pci(%02d:%02d.%d)\n",
		pdev->bn,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));

	err=pci_device_enable(pdev);
	if (err < 0) {
		printf("Failed to enable device\n");
		return err;
	}
	pci_set_master(pdev);
		pci_router=&i440fx_router_ops;

	prcw(pdev->bn,pdev->devfn,PCI_VENDOR_ID,&i440.f0.vid);
	prcw(pdev->bn,pdev->devfn,PCI_DEVICE_ID,&i440.f0.did);
	prcw(pdev->bn,pdev->devfn,PCI_COMMAND,&i440.f0.pcicmd);
	prcw(pdev->bn,pdev->devfn,PCI_STATUS,&i440.f0.pcists);
	prcb(pdev->bn,pdev->devfn,0x0d,&i440.f0.mlt);
	prcb(pdev->bn,pdev->devfn,PCI_HEADER_TYPE,&i440.f0.hdr);
	prcb(pdev->bn,pdev->devfn,0x0f,&i440.f0.bist);
	prcb(pdev->bn,pdev->devfn,0x3c,&i440.f0.intln);
	prcb(pdev->bn,pdev->devfn,0x3d,&i440.f0.intpn);
	prcw(pdev->bn,pdev->devfn,0x50,&i440.f0.pmccfg);
	prcb(pdev->bn,pdev->devfn,0x52,&i440.f0.deturbo);
	prcb(pdev->bn,pdev->devfn,0x53,&i440.f0.dbc);
	prcb(pdev->bn,pdev->devfn,0x54,&i440.f0.axc);
	prcw(pdev->bn,pdev->devfn,0x55,&i440.f0.dramr);
	prcb(pdev->bn,pdev->devfn,0x57,&i440.f0.dramc);
	prcb(pdev->bn,pdev->devfn,0x58,&i440.f0.dramt);
	for(i=0;i<7;i++)
		prcb(pdev->bn,pdev->devfn,0x59+i,&i440.f0.pam[i]);
	for(i=0;i<8;i++)
		prcb(pdev->bn,pdev->devfn,0x60+i,&i440.f0.drb[i]);
	prcb(pdev->bn,pdev->devfn,0x68,&i440.f0.fdhc);
	prcb(pdev->bn,pdev->devfn,0x70,&i440.f0.mtt);
	prcb(pdev->bn,pdev->devfn,0x71,&i440.f0.clt);
	prcb(pdev->bn,pdev->devfn,0x72,&i440.f0.smram);
	prcb(pdev->bn,pdev->devfn,0x90,&i440.f0.errcmd);
	prcb(pdev->bn,pdev->devfn,0x91,&i440.f0.errsts);
	prcb(pdev->bn,pdev->devfn,0x93,&i440.f0.trc);
	for(pin=0;pin<DEVPCI_LEGACY_IRQ_PINS;pin++) 
				i440.f0.pirqrc[pin]=pci_router->route(PIRQ_GET,pin+1);
	return 0;
}

int
I440FX_info(void)
{
	printf("\nVID=[%x] DID=[%x] COMMAND=[%x] STATUS=[%x] HDR=[%x]\n",
			i440.f0.vid,i440.f0.did,i440.f0.pcicmd,
			i440.f0.pcists,i440.f0.hdr);
	printf("INTLN=[%d] INTPIN=[%d] MLT=[%x] BIST=[%x]\n",
		i440.f0.intln, i440.f0.intpn, i440.f0.mlt,i440.f0.bist);
	printf("PM=[%x] DETURBO=[%x] DBC=[%x] AXC=[%x]\n",
			i440.f0.pmccfg,i440.f0.deturbo,i440.f0.dbc,i440.f0.axc);
	printf("DRAMR=[%x] DRAMC=[%x] DRAMT=[%x]\n",
				i440.f0.dramr,i440.f0.dramc,i440.f0.dramt);
	printf("PAM0=[%x] PAM1=[%x] PAM2=%x PAM3=%x PAM4=%x PAM5=[%x]\n",
		i440.f0.pam[0],i440.f0.pam[1],i440.f0.pam[2],
		i440.f0.pam[3],i440.f0.pam[4],i440.f0.pam[5]);
	printf("DRB0=[%x] DRB1=[%x] DRB2=[%x] DRB3=[%x]\n",
		i440.f0.drb[0],i440.f0.drb[1],i440.f0.drb[2],i440.f0.drb[3]);
	printf("DRB4=[%x] DRB5=[%x] DRB6=[%x] DRB7=[%x]\n",
		i440.f0.drb[4],i440.f0.drb[5],i440.f0.drb[6],i440.f0.drb[7]);
	printf("FDHC=[%x] MTT=[%x] CLT=[%x] SMRAM=[%x]\n",
			i440.f0.fdhc,i440.f0.mtt,i440.f0.clt,i440.f0.smram);
	printf("ERRCMD=[%x] ERRSTS=[%x] TRC=[%x]\n",
				i440.f0.errcmd,i440.f0.errsts,i440.f0.trc);
	return 0;
}

int 
I440FX_route(int cmd,int pirq,...)
{
	va_list	ap;
	u8_t	irq=0;

	DrvDebug(_PCI,5,"I440FX_route(cmd=%d, pirq=%d): unsupported, routing handled by southbridge\n", cmd,pirq);
	return -ENODEV;
}

static pcidevid_t PIIX4_pci_tbl[] = 
{
    { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371AB_0) },
    { 0, }
};

struct pci_driver PIIX4_driver = 
{
	"PIIX4",
	PIIX4_probe_pci,
	0,
	PIIX4_pci_tbl,
	0,
};

static pci_router_ops_t piix4_router_ops =
{
	"PIIX4",
	PIIX4_route,
	0x60
};

int 
PIIX4_init()
{
	int	PIIX4_have_pci=0;

	if (pci_devices == 0)
		pciinit();

	if (pci_register_driver(&PIIX4_driver))
		PIIX4_have_pci=1;
	else
		DrvDebug(_PCI,5,"PIIX4_init(): Not Found\n");

	return (PIIX4_have_pci) ? 0 : -ENODEV;
}

int
PIIX4_probe_pci(pcidev_t *pdev, pcidevid_t *ent)
{
	int	err=0, pin;

	DrvDebug(_PCI,5,"PIIX4_probe_pci(%02d:%02d.%d)\n",
		pdev->bn,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));

	err=pci_device_enable(pdev);
	if (err < 0) {
		printf("Failed to enable device\n");
		return err;
	}
	pci_set_master(pdev);
		pci_router=&piix4_router_ops;

	prcw(pdev->bn,pdev->devfn,PCI_VENDOR_ID,&piix4.f0.vid);
	prcw(pdev->bn,pdev->devfn,PCI_DEVICE_ID,&piix4.f0.did);
	prcw(pdev->bn,pdev->devfn,PCI_COMMAND,&piix4.f0.pcicmd);
	prcw(pdev->bn,pdev->devfn,PCI_STATUS,&piix4.f0.pcists);
	prcb(pdev->bn,pdev->devfn,PCI_HEADER_TYPE,&piix4.f0.hdr);
	prcb(pdev->bn,pdev->devfn,0x3c,&piix4.f0.intln);
	prcb(pdev->bn,pdev->devfn,0x3d,&piix4.f0.intpn);
	prcd(pdev->bn,pdev->devfn,0x40,&piix4.f0.pmba);
	prcd(pdev->bn,pdev->devfn,0x44,&piix4.f0.cnta);
	prcd(pdev->bn,pdev->devfn,0x48,&piix4.f0.cntb);
	prcd(pdev->bn,pdev->devfn,0x4c,&piix4.f0.gpictl);
	prcd(pdev->bn,pdev->devfn,0x54,&piix4.f0.devacta);
	prcd(pdev->bn,pdev->devfn,0x58,&piix4.f0.devactb);
	prcd(pdev->bn,pdev->devfn,0x5c,&piix4.f0.devresa);
	prcd(pdev->bn,pdev->devfn,0x60,&piix4.f0.devresb);

	/*** Pick up PIRQ settings ***/
	for(pin=0;pin<DEVPCI_LEGACY_IRQ_PINS;pin++) 
				piix4.f0.pirqrc[pin]=pci_router->route(PIRQ_GET,pin+1);

	prcd(pdev->bn,pdev->devfn,0x64,&piix4.f0.devresc);
	prcd(pdev->bn,pdev->devfn,0x50,&piix4.f0.devresd);
	prcd(pdev->bn,pdev->devfn,0x68,&piix4.f0.devrese);
	prcd(pdev->bn,pdev->devfn,0x6c,&piix4.f0.devresf);
	prcd(pdev->bn,pdev->devfn,0x70,&piix4.f0.devresg);
	prcd(pdev->bn,pdev->devfn,0x74,&piix4.f0.devresh);
	prcd(pdev->bn,pdev->devfn,0x78,&piix4.f0.devresi);
	prcd(pdev->bn,pdev->devfn,0x7c,&piix4.f0.devresj);
	prcd(pdev->bn,pdev->devfn,0x80,&piix4.f0.pmregmisc);
	prcd(pdev->bn,pdev->devfn,0x90,&piix4.f0.smbba);
	prcb(pdev->bn,pdev->devfn,0xd2,&piix4.f0.smbhstcfg);
	prcb(pdev->bn,pdev->devfn,0xd3,&piix4.f0.smbslvc);
	prcb(pdev->bn,pdev->devfn,0xd4,&piix4.f0.smbshdw1);
	prcb(pdev->bn,pdev->devfn,0xd5,&piix4.f0.smbshdw2);
	prcb(pdev->bn,pdev->devfn,0xd6,&piix4.f0.smbrev);
	prcd(pdev->bn,pdev->devfn,0x60,&pci_remaps);
	return 0;
}

int
PIIX4_info(void)
{
	printf("\nVID=[%x] DID=[%x] COMMAND=[%x] STATUS=[%x] HDR=[%x]\n",
		piix4.f0.vid,piix4.f0.did,piix4.f0.pcicmd,
		piix4.f0.pcists,piix4.f0.hdr);
	printf("INTLN=[%d] INTPIN=[%d] PMBA=[%x]\n",
		piix4.f0.intln, piix4.f0.intpn, piix4.f0.pmba);
	printf("CNTA=[%x] CNTB=[%x] GPICTL=[%x]\n",
		piix4.f0.cnta, piix4.f0.cntb, piix4.f0.gpictl);
	printf("DEVACTA=[%x] DEVACTB=[%x] DEVRESA=[%x] DEVRESB=[%x]\n",
		piix4.f0.devacta, piix4.f0.devactb, 
		piix4.f0.devresa, piix4.f0.devresb);
	printf("DEVRESC=[%x] DEVRESD=[%x] DEVRESE=[%x] DEVRESF=[%x]\n",
		piix4.f0.devresc, piix4.f0.devresd, 
		piix4.f0.devrese, piix4.f0.devresf);
	printf("DEVRESG=[%x] DEVRESH=[%x] DEVRESI=[%x] DEVRESJ=[%x]\n",
		piix4.f0.devresg, piix4.f0.devresh, 
		piix4.f0.devresi, piix4.f0.devresj);
	printf("PMREGMISC=[%x] SMBBA=[%x]\n",piix4.f0.pmregmisc,piix4.f0.smbba);
	printf("SMBHSTCFG=[%x] SMBSLVC=[%x] SMBSHDW1=[%x] SMBSHDW2=[%x]\n",
		piix4.f0.smbhstcfg, piix4.f0.smbslvc, 
		piix4.f0.smbshdw1, piix4.f0.smbshdw2);
	printf("SMBREV=[%x]\n",piix4.f0.smbrev);
	printf("PIRQRC0=[%x] PIRQRC1=[%x] PIRQRC2=[%x] PIRQRC3=[%x]\n",
	      	piix4.f0.pirqrc[0],piix4.f0.pirqrc[1],
		piix4.f0.pirqrc[2],piix4.f0.pirqrc[3]);
	return 0;
}

int 
PIIX4_route(int cmd,int pirq,...)
{
	va_list	ap;
	u8_t	irq; 
	int	i;

	switch(cmd) {
	case PIRQ_GET:
		prcb(Pbn,Pdevfn,pci_router->pirq_base+pirq-1,&irq);
		DrvDebug(_PCI,5,"PIIX4_route(GET, PIRQ%c) -> %d\n",
			'A'+pirq-1,irq);
		return (int)irq;

	case PIRQ_SET:
		va_start(ap,pirq);	
		i=va_arg(ap,int);	
		va_end(ap);
		irq=(u8_t)i&0xff;
		pwcb(Pbn,Pdevfn,pci_router->pirq_base+pirq-1,irq);
		DrvDebug(_PCI,5,"PIIX4_route(SET, PIRQ%c -> IRQ %d)\n",
			'A'+pirq-1,irq);
		return 0;
	default:
		printf("Unknown cmd %d\n",cmd);
	}
	return -1;
}

static pcidevid_t PIIX3_pci_tbl[] = 
{
    { PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_0) },
    { 0, }
};

struct pci_driver PIIX3_driver = 
{
	"PIIX3",
	PIIX3_probe_pci,
	0,
	PIIX3_pci_tbl,
	0
};

static pci_router_ops_t piix3_router_ops =
{
	"PIIX3",
	PIIX3_route,
	0x60
};

int 
PIIX3_init()
{
	int	PIIX3_have_pci=0;

	if (pci_devices == 0)
		pciinit();

	if (pci_register_driver(&PIIX3_driver))
		PIIX3_have_pci=1;
	else
		DrvDebug(_PCI,5,"PIIX3_init(): Not Found\n");

	return (PIIX3_have_pci) ? 0 : -ENODEV;
}

int
PIIX3_probe_pci(pcidev_t *pdev, pcidevid_t *ent)
{
	int	err=0, pin, i;

	DrvDebug(_PCI,5,"PIIX3_probe_pci(%02d:%02d.%d)\n",
		pdev->bn,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));

	err=pci_device_enable(pdev);
	if (err < 0) {
		printf("Failed to enable device\n");
		return err;
	}
		pci_router=&piix3_router_ops;

	prcw(pdev->bn,pdev->devfn,PCI_VENDOR_ID,&piix3.f0.vid);
	prcw(pdev->bn,pdev->devfn,PCI_DEVICE_ID,&piix3.f0.did);
	prcw(pdev->bn,pdev->devfn,PCI_COMMAND,&piix3.f0.pcicmd);
	prcw(pdev->bn,pdev->devfn,PCI_STATUS,&piix3.f0.pcists);
	prcb(pdev->bn,pdev->devfn,PCI_HEADER_TYPE,&piix3.f0.hdr);
	prcb(pdev->bn,pdev->devfn,0x3c,&piix3.f0.intln);
	prcb(pdev->bn,pdev->devfn,0x3d,&piix3.f0.intpn);
	prcb(pdev->bn,pdev->devfn,0x4c,&piix3.f0.iort);
	prcw(pdev->bn,pdev->devfn,0x4e,&piix3.f0.xbcs);

	/*** Pick up PIRQ settings ***/
	for(pin=0;pin<DEVPCI_LEGACY_IRQ_PINS;pin++) 
		piix3.f0.pirqrc[pin]=pci_router->route(PIRQ_GET,pin+1);

	prcb(pdev->bn,pdev->devfn,0x69,&piix3.f0.tom);
	prcw(pdev->bn,pdev->devfn,0x6a,&piix3.f0.mstat);
	for(i=0;i<2;i++)
		prcb(pdev->bn,pdev->devfn,0x70+i,&piix3.f0.mbirq[i]);
	for(i=0;i<2;i++)
		prcb(pdev->bn,pdev->devfn,0x76+i,&piix3.f0.mbdma[i]);
	prcw(pdev->bn,pdev->devfn,0x78,&piix3.f0.pcsc);
	prcb(pdev->bn,pdev->devfn,0x80,&piix3.f0.apicbase);
	prcb(pdev->bn,pdev->devfn,0x82,&piix3.f0.dlc);
	prcb(pdev->bn,pdev->devfn,0xa0,&piix3.f0.smicntl);
	prcw(pdev->bn,pdev->devfn,0xa2,&piix3.f0.smien);
	prcd(pdev->bn,pdev->devfn,0xa4,&piix3.f0.see);
	prcb(pdev->bn,pdev->devfn,0xa8,&piix3.f0.ftmr);
	prcw(pdev->bn,pdev->devfn,0xaa,&piix3.f0.smireq);
	prcb(pdev->bn,pdev->devfn,0xac,&piix3.f0.ctltmr);
	prcb(pdev->bn,pdev->devfn,0xad,&piix3.f0.cthtmr);
	prcd(pdev->bn,pdev->devfn,0x60,&pci_remaps);
	return 0;
}

int
PIIX3_info(void)
{
	printf("\nVID=[%x] DID=[%x] COMMAND=[%x] STATUS=[%x] HDR=[%x]\n",
		piix3.f0.vid, piix3.f0.did, piix3.f0.pcicmd,
		piix3.f0.pcists, piix3.f0.hdr);
	printf("INTLN=[%d] INTPIN=[%d] IORT=[%x] XBCS=[%x]\n",
		piix3.f0.intln, piix3.f0.intpn,piix3.f0.iort, piix3.f0.xbcs);
	printf("PIRQRCA=[%d] PIRQRCB=[%d] PIRQRCC=[%d] PIRWQCD=[%d]\n",
		piix3.f0.pirqrc[0], piix3.f0.pirqrc[1],
		piix3.f0.pirqrc[2], piix3.f0.pirqrc[3]);
	printf("TOM=[%x] MSTAT=[%x] MBIRQ0=[%x] MBIRQ1=[%x] MBDMA0=[%x]\n",
		piix3.f0.tom, piix3.f0.mstat, 
		piix3.f0.mbirq[0], piix3.f0.mbirq[1], piix3.f0.mbdma[0]);
	printf("MBDMA1=[%x] PCSC=[%x] APICBASE=[%x] DLC=[%x]\n",
		piix3.f0.mbdma[1],piix3.f0.pcsc,piix3.f0.apicbase,piix3.f0.dlc);
	printf("SMICNTL=[%x] SMIEN=[%x] SEE=[%x]\n",
		piix3.f0.smicntl, piix3.f0.smien, piix3.f0.see);
	printf("FTMR=[%x] SMIREQ=[%x] CTLTMR=[%x] CTHTMR=[%x]\n",
		piix3.f0.ftmr,piix3.f0.smireq,
		piix3.f0.ctltmr,piix3.f0.cthtmr);
	return 0;
}

int 
PIIX3_route(int cmd,int pirq,...)
{
	va_list	ap;
	u8_t	irq=0;
	int	i;

	switch(cmd) {
	case PIRQ_GET:
		prcb(Pbn,Pdevfn,pci_router->pirq_base+pirq-1,&irq);
		DrvDebug(_PCI,5,"PIIX3_route(GET, PIRQ%c) -> %d\n",
			'A'+pirq-1,irq);
		return irq;
	case PIRQ_SET:
		va_start(ap,pirq);	
		i=va_arg(ap,int);	
		va_end(ap);
		irq=(u8_t)i&0xff;
		pwcb(Pbn,Pdevfn,pci_router->pirq_base+pirq-1,irq);
		DrvDebug(_PCI,5,"PIIX3_route(SET, PIRQ%c -> IRQ %d)\n",
			'A'+pirq-1,irq);
		return 0;
	default:
		printf("Unknown cmd %d\n",cmd);
	}
	return 0;
}

static pcidevid_t VIA_pci_tbl[] = 
{
    { PCI_DEVICE(PCI_VENDOR_ID_VIA, PCI_DEVICE_ID_VIA_82C686) },
    { 0, }
};

struct pci_driver VIA_driver = 
{
	"VIA",
	VIA_probe_pci,
	0,
	VIA_pci_tbl,
	0
};

static pci_router_ops_t via_router_ops =
{
	"VIA",
	VIA_route,
	0x55
};

int 
VIA_init()
{
	int	VIA_have_pci=0;

	if (pci_devices == 0) {
		pciinit();
	}

	if (pci_register_driver(&VIA_driver))
		VIA_have_pci=1;
	else
		DrvDebug(_PCI,5,"VIA_init(): Not Found\n");

	return (VIA_have_pci) ? 0 : -ENODEV;
}

int
VIA_probe_pci(pcidev_t *pdev, pcidevid_t *ent)
{
	int	err=0, pin;

	DrvDebug(_PCI,5,"VIA_probe_pci(%02d:%02d.%d)\n",
		pdev->bn,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));

	err=pci_device_enable(pdev);
	if (err < 0) {
		printf("Failed to enable device\n");
		return err;
	}
	pci_set_master(pdev);
		pci_router=&via_router_ops;

	prcw(pdev->bn,pdev->devfn,PCI_VENDOR_ID,&via.f0.vid);
	prcw(pdev->bn,pdev->devfn,PCI_DEVICE_ID,&via.f0.did);
	prcw(pdev->bn,pdev->devfn,PCI_COMMAND,&via.f0.pcicmd);
	prcw(pdev->bn,pdev->devfn,PCI_STATUS,&via.f0.pcists);
	prcb(pdev->bn,pdev->devfn,PCI_HEADER_TYPE,&via.f0.hdr);
	prcb(pdev->bn,pdev->devfn,PCI_CLASS_REVISION,&via.f0.revid);
	prcb(pdev->bn,pdev->devfn,0x3c,&via.f0.intln);
	prcb(pdev->bn,pdev->devfn,0x3d,&via.f0.intpn);

	prcb(pdev->bn,pdev->devfn,0x40,&via.f0.isabuscontrol);
	prcb(pdev->bn,pdev->devfn,0x46,&via.f0.misccontrol1);
	prcb(pdev->bn,pdev->devfn,0x47,&via.f0.misccontrol2);
	prcb(pdev->bn,pdev->devfn,0x48,&via.f0.misccontrol3);
	prcb(pdev->bn,pdev->devfn,0x4a,&via.f0.ideirqroute);
	prcb(pdev->bn,pdev->devfn,0x54,&via.f0.pciedgelevel);
	prcb(pdev->bn,pdev->devfn,0x55,&via.f0.pciinta);
	prcb(pdev->bn,pdev->devfn,0x56,&via.f0.pciintbc);
	prcb(pdev->bn,pdev->devfn,0x57,&via.f0.pciintd);
	prcb(pdev->bn,pdev->devfn,0x58,&via.f0.apicirq);
	prcb(pdev->bn,pdev->devfn,0x85,&via.f0.extfunc);

	/*** Pick up PIRQ settings ***/
	for(pin=0;pin<DEVPCI_LEGACY_IRQ_PINS;pin++) 
		via.f0.pirqrc[pin]=pci_router->route(PIRQ_GET,pin+1);

	{ 
	u16_t irq1,irq2;
	prcw(Pbn,Pdevfn,0x44,&irq1);
	prcw(Pbn,Pdevfn,0x46,&irq2);
	printf("irq1=%x irq2=%x\n",irq1,irq2);
	}
	prcd(pdev->bn,pdev->devfn,0x60,&pci_remaps);
	return 0;
}

int
VIA_info(void)
{
	int	i;

	printf("%02x %s\n",
		via.f0.revid,
		(via.f0.revid&0x0f)== 1 ? "VT82C686A" : "VT82C686");

	printf("\nVID=[%x] DID=[%x] COMMAND=[%x] STATUS=[%x] HDR=[%x]\n",
		via.f0.vid, via.f0.did, via.f0.pcicmd,
		via.f0.pcists, via.f0.hdr);
	printf("INTLINE=[%d] INTPIN=[%d]\n",
		via.f0.intln, via.f0.intpn);
	printf("MISC1=%08x MISC2=%08x MISC3=%08x\n",
		via.f0.misccontrol1,via.f0.misccontrol2,via.f0.misccontrol3);
	printf("IDEIRQROUTE=%08x PCIEDGELVL=%08x\n",
		via.f0.ideirqroute, via.f0.pciedgelevel);
	printf("PCIINTA=%08x PCIINTBC=%08x PCIINTD=%08x\n",
		via.f0.pciinta,via.f0.pciintbc,via.f0.pciintd);
	printf("APICIRQ=%08x EXTFUNC=%08x\n",
		via.f0.apicirq, via.f0.extfunc);

	printf("ISABUSCTRL=[%x] PCIEDGELEVEL=[%x] APICIRQ=[%x]\n",
		via.f0.isabuscontrol, 
		via.f0.pciedgelevel,
		via.f0.apicirq);
	printf("INTA=[%x] INTB=[%x] INTC=[%x] INTD=[%x]\n",
		via.f0.pciinta,
		LoNibble(via.f0.pciintbc),
		HiNibble(via.f0.pciintbc),
		via.f0.pciintd);

	for(i=0;i<4;i++)
	{
		if (via.f0.pciedgelevel & (1<<i))
			printf("PIRQ%c Edge\n",'A'+i);
		else
			printf("PIRQ%c Level\n",'A'+i);
	}
	printf("\n");
	return 0;
}

int 
VIA_route(int cmd,int pirq,...)
{
	va_list	ap;
	u8_t	irq; 
	int	i;

	switch(cmd) {
	case PIRQ_GET:
		prcb(Pbn,Pdevfn,pci_router->pirq_base+pirq-1,&irq);
		DrvDebug(_PCI,5,"VIA_route(GET, PIRQ%c) -> %d\n",
			'A'+pirq-1,irq&0x0f);
		return (irq & 0x80) ? PCI_INVALID_IRQ
				    : (int)(irq & 0x0f);

	case PIRQ_SET:
		va_start(ap,pirq);
		i=va_arg(ap,int);
		va_end(ap);	

		pwcb(Pbn,Pdevfn,pci_router->pirq_base+pirq-1,(u8_t)(i&0x0f));
		DrvDebug(_PCI,5,"VIA_route(SET, PIRQ%c -> IRQ %d)\n",
			'A'+pirq-1,i&0x0f);
		return 0;
	default:
		printf("Unknown cmd %d\n",cmd);
	}
	return -1;
}

int 
PCI_route(int cmd,int pirq,...)
{
	u8_t	irq=0; 
	va_list ap;
	int	i, r;

	r = ((pci_router != 0) ? pci_router->pirq_base : 0x60) + pirq-1;

	switch(cmd) {
	case PIRQ_GET:
		prcb(Pbn,Pdevfn,r,&irq);
		irq &= 0x0f;
		DrvDebug(_PCI,5,"PCI_route(GET, PIRQ%c) -> %d\n",
			'A'+pirq-1,irq);
		return (int)irq;

	case PIRQ_SET:
		va_start(ap,pirq);
		i=va_arg(ap,int);
		va_end(ap);
		prcb(Pbn,Pdevfn,r,&irq);
		irq = (u8_t)(i & 0x0f);
		pwcb(Pbn,Pdevfn,r,irq);
		DrvDebug(_PCI,5,"PCI_route(SET, PIRQ%c -> IRQ %d)\n",
			'A'+pirq-1,irq);
		return 0;
	default:
		printf("Unknown cmd %d\n",cmd);
	}
	return -1;
}

int
pci_slot_get_pirq(u8_t devfn, int irq_num)
{
	int	slot_addend;

	slot_addend = (devfn >> 3) - 1;
	return (irq_num+slot_addend) & 3;
}

int
pci_slot_get_apic_pirq(u8_t devfn, int irq_num)
{
	return (irq_num + (devfn>>3)) & 7;
}

int
PCI_get_interrupt(pcidev_t *pdev)
{
	int	pirq;

	DrvDebug(_PCI,5,"PCI_get_interrupt(IRQ=%d, INT#%c)\n",pdev->irq,
						     IntPin(pdev->intpin));

	if (pdev->intpin == 0 || pdev->intpin > 4) {
		printf("PCI device does not specify interrupt pin\n");
		return pdev->irq;
	}	

	pirq = ((int)((pdev->intpin-1) + PCI_SLOT(pdev->devfn)) % 4) ;

	DrvDebug(_PCI,5,"devfn=%04x [%c,irq=%d] pirq=%d maps to IRQ%d\n", 
		pdev->devfn,
		IntPin(pdev->intpin),
		pdev->irq,
		pirq,
		pci_remaps[pirq]);

	if (pci_remaps[pirq] == 0x00) {
		return pdev->irq;
	} else
	if (pci_remaps[pirq] >= 0x80) {
		printf("pci_get_intr(): Not mapped - remapping\n");

		if (pdev->irq == 0xff) {
			return PCI_INVALID_IRQ;
		}
		return pdev->irq;	
	}
	return pci_remaps[pirq];
}

void
show_irqroute()
{
	int	pin;
	u8_t 	mbirq[2];
	int	i;
	char	*msg;

	printf("PCI interrupt router at: %02X:%02X.%X\n",
		Pbn,PCI_SLOT(Pdevfn),PCI_FUNC(Pdevfn));

	for(pin=0;pin<DEVPCI_LEGACY_IRQ_PINS;pin++)
	{
		u8_t irq;

		irq=pci_router->route(PIRQ_GET,pin+1);
		if (irq & 0x80)
			printf("PIRQ%c disabled\n",IntPin(pin+1));
		else
			printf("PIRQ%c -> IRQ%d\n",IntPin(pin+1), irq & 0xf);
	}
	printf("\n");

	printf("MBIRQ:\n");
	for(i=0;i<2;i++)
	{
		prcb(Pbn,Pdevfn,0x70+i,&mbirq[i]);
		if (mbirq[i] & 0x80)
			printf("[%d] disabled [%x]\n",i,mbirq[i]);
		else
		{
			msg=(mbirq[i] & 0x40)?"Shared" : "Not-Shared";
			printf("[%d] %s %d\n",i,msg,mbirq[i]&0xf);
		}
	}
	printf("\n");
}

void
show_pirq()
{
	int	pin;

	printf("PCI IRQ levels:\n");
	for(pin=1;pin<DEVPCI_LEGACY_IRQ_PINS+1;pin++)
	{
		printf(" PIRQ%c: ",IntPin(pin));
		if (pci_remaps[pin-1] & 0x80)
			printf("disabled\n");
		else
			printf("%d\n",pci_remaps[pin-1]);
	}
	printf("\n");
}

/*
 * The format of the ELCR is simple: It is a 16-bit bitmap where bit 0 controls
 * IRQ 0, bit 1 controls IRQ 1, etc.  If the bit is zero, the associated IRQ
 * is edge triggered. If the bit is one, the IRQ is level triggered.
 */
void
elcr_show()
{
	int	i;

	printf("ELCR Found. ISA IRQs programmed as (%04x):\n",elcr_status);
	for(i=0; i<16; i++) 
		printf(" %2d",i);
	printf("\n");

	for(i=0; i<16; i++) 
	{
		if (elcr_status & ELCR_MASK(i))
			printf("  L");
		else
			printf("  E");
	}
	printf("\n");
}

int
elcr_probe()
{
	elcr_status=inb(ELCR_PORT) | inb(ELCR_PORT+1) << 8;
	if ((elcr_status & (ELCR_MASK(0) | ELCR_MASK(1) | ELCR_MASK(2) |
			    ELCR_MASK(8) | ELCR_MASK(13))) != 0) {
		DrvDebug(_PCI,5,"elcr_probe(): Not Found\n");
		return ENXIO;
	}
	elcr_found=1;
	return 0;
}

enum intr_trigger
elcr_read_trigger(int irq)
{
	if (irq < 0 || irq >= NUM_ISA_INTERRUPTS) {
		printf("elcr_read_trigger: IRQ %d out of range\n",irq);
		return INTR_TRIGGER_INVALID;
	}

	if (elcr_status & ELCR_MASK(irq))
		return INTR_TRIGGER_LEVEL;
	else
		return INTR_TRIGGER_EDGE;
}

void
elcr_write_trigger(int irq, enum intr_trigger trigger)
{
	int	new_status;

	if (irq < 0 || irq >= NUM_ISA_INTERRUPTS) {
		printf("elcr_write_trigger: IRQ %d out of range\n",irq);
		return;
	}

	if (trigger == INTR_TRIGGER_LEVEL)
		new_status = elcr_status | ELCR_MASK(irq);
	else
		new_status = elcr_status & ~ELCR_MASK(irq);

	if (new_status == elcr_status) return;

	elcr_status = new_status;
	if (irq >= 8)
		outb(ELCR_PORT+1, elcr_status>>8);
	else
		outb(ELCR_PORT, elcr_status & 0xff);
}

void
myset_elt(int irq, int mode)
{
	int i, port, bit;

	DrvDebug(_PCI,5,"myset_elt(%d,%d)\n",irq,mode);

	if (irq < 0 || irq >= NUM_ISA_INTERRUPTS) {
		printf("myset_elt: IRQ %d out of range\n",irq);
		return;
	}

	switch (irq) {
	case 0:
	case 1:
	case 2:
	case 13:
		mode = EDGE_TRIG;
		break;
	}
	if (irq > 7) {
		irq -= 8;
		port = ELCR_PORT1;
	} else
		port = ELCR_PORT0;

	bit = 1 << irq;

	i = inb(port);
	i &= ~bit;
	i |= (mode ? bit : 0);
	outb(port, i);

	if (port == ELCR_PORT0) {
		elcr_status = (elcr_status & 0xff00) | (i & 0x00ff);
	} else {
		elcr_status = (elcr_status & 0x00ff) | ((i & 0x00ff) << 8);
	}
	elcr_show();
		
	return;
}

#define IMASK(Pri,Pic)	iplmask[Pic+NPIC*Pri]

int
pci_probe_chipsets(void)
{
	(void)I440FX_init();
	(void)PIIX3_init();
	(void)PIIX4_init();
	(void)VIA_init();
	return 0;
}
