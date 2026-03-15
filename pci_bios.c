/*
 * pci_bios.c
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/immu.h>

#include "pci.h"
#include "pci_bios.h"
#include "pci_funcs.h"

int	verbose_bios = 0;

/*
 * pir_router_found set to 1 once a valid $PIR table has been parsed
 * and Pbn/Pdevfn have been loaded.  Routing functions can check this
 * rather than testing (Pbn == 0 && Pdevfn == 0), which is ambiguous
 * since bus 0 device 0 is a legitimate router location.
 */
int	pir_router_found = 0;

u64_t
U64(u32_t low, u32_t high)
{
	u64_t	self;

	self.l = low;
	self.h = high;
	return self;
}

char *
Str(char *ptr, int n)
{
	static char buf[32];
	int i;

	if (n >= sizeof(buf))
		n=sizeof(buf)-1;

	memcpy(buf,ptr,n);
	buf[i] = 0;
	return buf;
}

void
pir_slot_number(u8_t code)
{
	if (code == 0) printf(" on-board");
	else           printf(" slot %d", code);
}

void
pir_link_bitmap(char letter, PIR_intpin_t *p)
{
	if (p->link == 0) return;

	printf("\t\tINT%c: LINK 0x%02x, IRQ Bitmap", letter, p->link);
	pci_print_irqmask(p->irqs);
	printf("\n");
}

void
pir_bios(void *addr)
{
	int		 i, n;
	u8_t		*p = (u8_t *)addr;
	extern u8_t	 Pbn, Pdevfn;
	PIR_table_t	*ptbl = (PIR_table_t *)addr;
	PIR_header_t	*phdr = &ptbl->pt_header;
	PIR_entry_t	*pent;

	DrvDebug(_PCI,5,"pir_bios()\n");

	if (!checksum(addr, phdr->ph_length)) return;

	/*
	 * Record the router bus/devfn unconditionally these globals are
	 * needed by all PIRQ routing functions regardless of verbosity.
	 */
	Pbn    = phdr->ph_router_bus;
	Pdevfn = phdr->ph_router_dev_fn;
	pir_router_found = 1;

	if (verbose_bios) {
		printf("PCI Interrupt Routing %d.%d present.\n", p[5], p[4]);
		printf("\tRouter Device: %02x.%02x.%1x\n",
			phdr->ph_router_bus,
			PCI_SLOT(phdr->ph_router_dev_fn),
			PCI_FUNC(phdr->ph_router_dev_fn));

		printf("\tExclusive IRQs:");
		pci_print_irqmask(phdr->ph_pci_irqs);
		printf("\n");

		if (phdr->ph_router_vendor != 0)
			printf("\tCompatible Router: %04x:%04x\n",
				phdr->ph_router_vendor,
				phdr->ph_router_device);

		if (phdr->ph_miniport != 0)
			printf("\tMiniport Data: 0x%8x\n", phdr->ph_miniport);

		n = (int)(phdr->ph_length - sizeof(PIR_header_t)) /
		          sizeof(PIR_entry_t);

		for (i = 0; i < n; i++) {
			pent = &ptbl->pt_entry[i];
			printf("\tDevice: %2x:%2x,",
				pent->pe_bus, pent->pe_devfn);
			pir_slot_number(pent->pe_slot);
			printf("\n");
			pir_link_bitmap('A', &pent->pe_intpin[0]);
			pir_link_bitmap('B', &pent->pe_intpin[1]);
			pir_link_bitmap('C', &pent->pe_intpin[2]);
			pir_link_bitmap('D', &pent->pe_intpin[3]);
		}
	}
	pci_pir_parse((u32_t)addr);
}

void
sm_bios(void *addr)
{
	u8_t	major, minor, revision;
	int	length;

	length   = ADDR2EPS(addr)->length;
	major    = ADDR2EPS(addr)->major_version;
	minor    = ADDR2EPS(addr)->minor_version;
	revision = ADDR2EPS(addr)->BCD_revision;

	if (length != 0x1f) {
		if (length == 0x1e && major == 2 && minor == 1)
			length = 0x1f;
		else {
			DrvDebug(_PCI,5,"sm_bios() Not Found\n");
			return;
		}
	}
	DrvDebug(_PCI,5,"System Management Bios %d.%d\n", major, minor);
}

void
bios32(void *addr)
{
	u8_t	*p, rev;
	size_t	 len;
	u32_t	 iaddr;


	p    = (u8_t *)addr;
	len  = p[0x09] << 4;

	if (len < 0x0a || !checksum(p, len)) {
		DrvDebug(_PCI,5,"bios32() Not Found\n");
		return;
	}

	rev   = p[0x08];
	iaddr = DWORD(p + 0x04);

	if (verbose_bios) {
		printf("BIOS32 Service Directory present.\n");
		printf("\tRevision: %d\n", rev);
		printf("\tCalling Interface Address: 0x%08x\n", iaddr);
	}
}

char *
pnp_event_notification(u8_t code)
{
	static char *notification[] = {
		"Not supported",
		"Polling",
		"Asyncronous",
		"Unknown",
	};
	return notification[code];
}

void
pnp_bios(void *addr)
{
	u8_t	*p;
	size_t	 len;


	p   = (u8_t *)addr;
	len = p[0x05] << 4;

	if (len < 0x20 || !checksum(p, len)) {
		DrvDebug(_PCI,5,"pnp_bios() Not Found\n");
		return;
	}

	if (verbose_bios) {
		printf("PNP BIOS %d.%d present\n", p[0x04] >> 4, p[0x04] & 0xf);
		printf("\tEvent Notification: %s\n",
			pnp_event_notification(WORD(p + 0x06) & 0x03));
		if ((WORD(p + 0x06) & 0x03) == 0x01)
			printf("\tEvent Notification Flag Address: 0x%8x\n",
				DWORD(p + 0x09));
		printf("\tReal Mode 16-bit Code Address: %04x-%04x\n",
			WORD(p + 0x0f), WORD(p + 0x0d));
		printf("\tReal Mode 16-bit Data Address: %04x-0000\n",
			WORD(p + 0x1b));
		printf("\t16-bit Protected Mode Code Address: 0x%08x\n",
			DWORD(p + 0x13) + WORD(p + 0x11));
		printf("\t16-bit Protected Mode Data Address: 0x%08x\n",
			DWORD(p + 0x1d));
		if (DWORD(p + 0x17) != 0)
			printf("\tOEM Device Identifier: %c%c%c%02x%02x%02x\n",
				0x40 + ((p[0x17] >> 2) & 0x1f),
				0x40 + ((p[0x17] & 0x03) << 3) +
				       ((p[0x18] >> 5) & 0x07),
				0x40 + (p[0x18] & 0x1f),
				p[0x19], p[0x1a]);
	}
}

/*
 * dmi_bios() Legacy DMI structure decoder.
 *
 * The DMI entry point structure is exactly 0x0f bytes.  The length
 * field lives at p[0x05] (same offset as the PnP length field) and
 * should equal 0x0f for a well-formed table.  We read it from the
 * data rather than hardcoding it so that the bounds check is not
 * trivially false.
 *
 * Bug fixed: the original assigned len=0x0f and then tested
 * (len < 0x0f), which is always false, meaning any garbage in the
 * BIOS range that matched the "_DMI_" signature would pass validation
 * unchecked.
 */
void
dmi_bios(void *addr)
{
	u8_t	*p;
	size_t	 len;

	p   = (u8_t *)addr;
	len = p[0x05];

	if (len < 0x0f || !checksum(p, len)) {
		DrvDebug(_PCI,5,"dmi_bios() Not Found\n");
		return;
	}

	if (verbose_bios) {
		printf("Legacy DMI %d.%d present.\n",
			p[0x0e] >> 4, p[0x0e] & 0x0f);
		printf("\tStructure Table Length: %d bytes\n", WORD(p + 0x06));
		printf("\tStructure Table Address: 0x%08x\n", DWORD(p + 0x08));
		printf("\tNumber of Structures: %d\n", WORD(p + 0x0c));
	}
}

/*
 * acpiCheckHeader() validate an ACPI SDT header signature and checksum.
 *
 * Note: this function has no call sites in the current codebase.
 * It is retained as it may be called from external code, but should
 * be declared in a header if so.
 */
u32_t
acpiCheckHeader(u32_t *ptr, char *sig)
{
	u8_t	*checkPtr, check = 0;
	int	 len = *(ptr + 1);
	char	*cp  = (char *)ptr;

	printf("Signature: %c%c%c%c\n", cp[0], cp[1], cp[2], cp[3]);
	printf("Len: %d\n", len);

	if (bcmp(ptr, sig, 4) != 0) {
		printf("Not [%s]\n", sig);
		return -1;
	}

	checkPtr = (u8_t *)ptr;
	while (0 < len--)
		check += *checkPtr++;

	printf("check=%02x\n", check);
	return (check == 0) ? 0 : -1;
}

void
acpi_bios(void *addr)
{
	RSDP_t	*rsdp = addr;
	RSDT_t	*rsdt = 0;

	DrvDebug(_PCI,5,"acpi_bios()\n");

	if (!checksum((u8_t *)rsdp, (rsdp->Revision < 2) ? sizeof(RSDP_t)
	                                         : sizeof(XSDP_t))) {
		printf("No ACPI (revision %d)\n", rsdp->Revision);
		return;
	}

	switch (rsdp->Revision) {
	case 0:
		printf("ACPI 1.0\n");
		rsdt = (RSDT_t *)phystokv((caddr_t)rsdp->rsdtAddress);
		break;
	case 2:
		printf("ACPI 2.0\n");
		break;
	}

	printf("OEMID: %s\n", Str(rsdp->OEMID, 6));
	if (!rsdt) return;

	printf("Signature:  %s\n",  Str(rsdt->hdr.Signature, 4));
	printf("Length:     %d\n",  rsdt->hdr.Length);
	printf("Revision:   %d\n",  rsdt->hdr.Revision);
	printf("OEMID:      %s\n",  Str(rsdt->hdr.OEMID, 6));
	printf("OEMTableID: %s\n",  Str(rsdt->hdr.OEMTableID, 8));
	printf("CreatorID:  %x\n",  rsdt->hdr.CreatorID);
	printf("CreatorRev: %x\n",  rsdt->hdr.CreatorRev);
}

int
checksum(u8_t *buffer, size_t length)
{
	u8_t sum = 0, *i = buffer;

	buffer += length;
	for (; i < buffer; sum += *(i++))
		;
	return (sum == 0);
}

u32_t
bios_sigsearch(u32_t start, char *sig, int siglen, int paralen, int sigofs)
{
	u8_t	*sp, *end;

	/* Compute the starting address */
	if ((start >= BIOS_START) && (start <= (BIOS_START + BIOS_SIZE)))
		sp = (u8_t *)BIOS_PADDRTOVADDR(start);
	else
	if (start == 0)
		sp = (u8_t *)BIOS_PADDRTOVADDR(BIOS_START);
	else
		return 0;

	/* Compute the end address */
	end = (u8_t *)BIOS_PADDRTOVADDR(BIOS_START + BIOS_SIZE);

	/* Loop search */
	while ((sp + sigofs + siglen) < end) {
		if (!bcmp(sp + sigofs, sig, siglen))
			return (u32_t)BIOS_VADDRTOPADDR(sp);
		sp += paralen;
	}
	return 0;
}

struct bios_entry {
	char	*sig;
	u32_t	 start;
	u32_t	 step;
	u32_t	 off;
	void	(*decode)(void *);
};

struct bios_entry bios_entries[] = {
    { "_SM3_",    SMBIOS3_START, SMBIOS3_STEP,  SMBIOS3_OFF, sm_bios   },
    { "_SM_",     SMBIOS_START,  SMBIOS_STEP,   SMBIOS_OFF,  sm_bios   },
    { "$PIR",     PIR_START,     PIR_STEP,      PIR_OFF,     pir_bios  },
    { "_PIR",     PIR_START,     PIR_STEP,      PIR_OFF,     pir_bios  },
    { "_PRT",     PIR_START,     PIR_STEP,      PIR_OFF,     pir_bios  },
    { "_32_",     BIOS32_START,  BIOS32_STEP,   BIOS32_OFF,  bios32    },
    { "$PnP",     PNP_START,     PNP_STEP,      PNP_OFF,     pnp_bios  },
    { "_DMI_",    DMI_START,     DMI_STEP,      DMI_OFF,     dmi_bios  },
    { "_SYSID_",  SYSID_START,   SYSID_STEP,    SYSID_OFF,   0         },
    { "_MP_",     MP_START,      MP_STEP,       MP_OFF,      0         },
    { "$SMB",     SMAPI_START,   SMAPI_STEP,    SMAPI_OFF,   0         },
    { "RSD PTR ", ACPI_START,    ACPI_STEP,     ACPI_OFF,    acpi_bios },
    { NULL,       0,             0,             0,           0         }
};

void
scan_bios()
{
	u32_t		    addr;
	int		    siglen;
	struct bios_entry  *ptr;

	DrvDebug(_PCI,5,"scan_bios()\n");

	for (ptr = &bios_entries[0]; ptr->sig; ptr++) {
		siglen = strlen(ptr->sig);
		if (addr = bios_sigsearch(ptr->start, ptr->sig, siglen,
		                          ptr->step, ptr->off)) {
			if (ptr->decode)
				(*ptr->decode)((void *)BIOS_PADDRTOVADDR(addr));
		}
	}
}
