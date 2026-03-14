/*
 * pci.h
 *
 * PCI defines and function prototypes
 */
#ifndef PCI_H
#define PCI_H

#include <sys/types.h>

typedef unsigned long ptrsize_t;

/*
 * offsetof: use size_t cast, not int, so the value is unsigned and
 * wide enough on any target.  The (size_t)(&((Type *)0)->Elem) idiom
 * is the traditional implementation; compilers that define __builtin_offsetof
 * should use that instead.
 */
#define offsetof(Type,Elem) \
	((size_t)(&((Type *)0)->Elem))

/*
 * CONFIG_ADDRESS format (CF8h):
 * 31    = Enable Config Space
 * 30-24 = Reserved
 * 23-16 = Bus Number
 * 15-11 = Device Number
 * 10-8  = Function Number
 *  7-2  = Register Number
 *  1-0  = 0 (must be zero)
 */

#include "plist.h"

#define SVR4
#if defined(SVR4) && defined(i386)
#  ifndef SI86IOPL
#  define SET_IOPL()   sysi86(SI86V86, V86SC_IOPL, PS_IOPL)
#  define RESET_IOPL() sysi86(SI86V86, V86SC_IOPL, 0)
#  else
#  define SET_IOPL()   sysi86(SI86IOPL, 3)
#  define RESET_IOPL() sysi86(SI86IOPL, 0)
#  endif
#endif

#define powerof2(x)	((((x)-1) & (x)) == 0)

/* Nibble accessors parenthesise argument to avoid operator-precedence bugs */
#define LoNibble(X)	((X) & 0x0f)
#define HiNibble(X)	(((X) & 0xf0) >> 4)

/*
 * WORD/DWORD: read unaligned little-endian values from a byte pointer.
 */
#define WORD(p) \
	((u16_t)( (u16_t)((u8_t *)(p))[0]         \
	        | (u16_t)((u8_t *)(p))[1] << 8 ))

#define DWORD(p) \
	((u32_t)( (u32_t)((u8_t *)(p))[0]          \
	        | (u32_t)((u8_t *)(p))[1] << 8      \
	        | (u32_t)((u8_t *)(p))[2] << 16     \
	        | (u32_t)((u8_t *)(p))[3] << 24 ))

#define QWORD(p)	(U64(DWORD(p), DWORD((u8_t *)(p) + 4)))

#define IORESOURCE_TYPE_BITS	0x00001f00
#define IORESOURCE_IO		0x00000100
#define IORESOURCE_MEM		0x00000200

#define DEVPCI_LEGACY_IRQ_PINS	4

#define CFG_ADDR	0x0cf8
#define CFG_DATA	0x0cfc
#define PCI_EN		0x80000000U

/*
 * ALIGN: produces a mask, not an address name is misleading but kept for
 * source compatibility. 
 */
#define ALIGN(x)	(~(sizeof(x) - 1))

#define PCI_ADDR(B, DF) \
	((u32_t)(PCI_EN                          \
	       | (((u32_t)(u8_t)(B))  << 16)     \
	       | (((u32_t)(u8_t)(DF)) <<  8)))

enum {
	PCI_RD,
	PCI_WR
};

#define _VD(vend, dev)    (u16_t)(vend), (u16_t)(dev)
#define _SS(sv, sd)       (u16_t)(sv),   (u16_t)(sd)
#define _CM(class, mask)  (u16_t)(class), (u16_t)(mask)

#define PCI_DEVICE(vendor, dev) \
		_VD(vendor, dev),              \
		_SS(PCI_ANY_ID, PCI_ANY_ID),   \
		_CM(0, 0),                     \
		0

#define PCIDEBUG	if (pci_debug) printf

#define BAR_ISIO(x)	(((x) & 0x01) == 1)
#define IOMASK		((u32_t)0xfffffffc)
#define MEMMASK		((u32_t)0xfffffff0)
#define PCI_ANY_ID	((u16_t)(~0))

#define pci_resource_start(dev, bar)	((dev)->resource[(bar)].start)
#define pci_resource_end(dev, bar)	((dev)->resource[(bar)].end)
#define pci_resource_flags(dev, bar)	((dev)->resource[(bar)].flags)

#include "pci_ids.h"

#define PCI_INVALID_IRQ		0xff
#define PCI_FUNCMAX		7
#define PCI_INTERRUPT_VALID(x)	((x) != PCI_INVALID_IRQ)

/*
 * PCI configuration space standard header (first 64 bytes).
 */
#define PCI_VENDOR_ID		0x00	/* 16 bits */
#define PCIV_INVALID		0xffff

#define PCI_DEVICE_ID		0x02	/* 16 bits */
#define PCI_COMMAND		0x04	/* 16 bits */
#define PCI_COMMAND_IO		0x001	/* Enable response in I/O space */
#define PCI_COMMAND_MEMORY	0x002	/* Enable response in Memory space */
#define PCI_COMMAND_MASTER	0x004	/* Enable bus mastering */
#define PCI_COMMAND_SPECIAL	0x008	/* Enable response to special cycles */
#define PCI_COMMAND_INVALIDATE	0x010	/* Use memory write and invalidate */
#define PCI_COMMAND_VGA_PALETTE	0x020	/* Enable palette snooping */
#define PCI_COMMAND_PARITY	0x040	/* Enable parity checking */
#define PCI_COMMAND_WAIT	0x080	/* Enable address/data stepping */
#define PCI_COMMAND_SERR	0x100	/* Enable SERR */
#define PCI_COMMAND_FAST_BACK	0x200	/* Enable back-to-back writes */

#define PCI_STATUS		0x06	/* 16 bits */
#define PCI_STATUS_66MHZ	0x0020	/* Support 66 MHz PCI 2.1 bus */
#define PCI_STATUS_UDF		0x0040	/* Support User Definable Features */
#define PCI_STATUS_FAST_BACK	0x0080	/* Accept fast-back to back */
#define PCI_STATUS_PARITY	0x0100	/* Detected parity error */
#define PCI_STATUS_DEVSEL_MASK		0x0600	/* DEVSEL timing */
#define PCI_STATUS_DEVSEL_FAST		0x0000
#define PCI_STATUS_DEVSEL_MEDIUM	0x0200
#define PCI_STATUS_DEVSEL_SLOW		0x0400
#define PCI_STATUS_SIG_TARGET_ABORT	0x0800	/* Set on target abort */
#define PCI_STATUS_REC_TARGET_ABORT	0x1000	/* Master ack of target abort */
#define PCI_STATUS_REC_MASTER_ABORT	0x2000	/* Set on master abort */
#define PCI_STATUS_SIG_SYSTEM_ERROR	0x4000	/* Set when we drive SERR */
#define PCI_STATUS_DETECTED_PARITY	0x8000	/* Set on parity error */

#define PCI_CLASS_REVISION	0x08	/* High 24 bits class, low 8 revision */
#define PCI_REVISION_ID		0x08	/* Revision ID */
#define PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE	0x0a	/* Device class */

#define PCI_CACHE_LINE_SIZE	0x0c	/* 8 bits */
#define PCI_LATENCY_TIMER	0x0d	/* 8 bits */
#define PCI_HEADER_TYPE		0x0e	/* 8 bits */
#define PCIM_HDRTYPE		0x7f
#define PCIM_MFDEV		0x80
#define PCI_HEADER_TYPE_NORMAL		0
#define PCI_HEADER_TYPE_BRIDGE		1
#define PCI_HEADER_TYPE_CARDBUS		2
#define PCI_MAXHDRTYPE			2

#define PCI_BIST		0x0f	/* 8 bits */
#define PCI_BIST_CODE_MASK	0x0f	/* Return result */
#define PCI_BIST_START		0x40	/* 1 to start BIST, 2 secs or less */
#define PCI_BIST_CAPABLE	0x80	/* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing 0xffffffff to the register
 * and reading it back only 1-bits are decoded.
 */
#define PCI_BASE_ADDRESS_0		0x10	/* 32 bits */
#define PCI_BASE_ADDRESS_1		0x14	/* 32 bits */
#define PCI_BASE_ADDRESS_2		0x18	/* 32 bits */
#define PCI_BASE_ADDRESS_3		0x1c	/* 32 bits */
#define PCI_BASE_ADDRESS_4		0x20	/* 32 bits */
#define PCI_BASE_ADDRESS_5		0x24	/* 32 bits */
#define PCI_BASE_ADDRESS_SPACE		0x01	/* 0 = memory, 1 = I/O */
#define PCI_BASE_ADDRESS_SPACE_IO	0x01
#define PCI_BASE_ADDRESS_SPACE_MEMORY	0x00

#define PCI_BASE_ADDRESS_MEM_TYPE_MASK	0x06
#define PCI_BASE_ADDRESS_MEM_TYPE_32	0x00	/* 32-bit address */
#define PCI_BASE_ADDRESS_MEM_TYPE_1M	0x02	/* Below 1M */
#define PCI_BASE_ADDRESS_MEM_TYPE_64	0x04	/* 64-bit address */
#define PCI_BASE_ADDRESS_MEM_PREFETCH	0x08	/* Prefetchable */
#define PCI_BASE_ADDRESS_MEM_MASK	(~0x0fU)
#define PCI_BASE_ADDRESS_IO_MASK	(~0x03U)
/* Bit 1 is reserved when address_space = 1 */

/*
 * Subsystem registers: PCI spec 2.1 table 6-2.
 * 0x2c = Subsystem Vendor ID (16 bits)
 * 0x2e = Subsystem ID        (16 bits)
 *
 * The original header had these two names swapped.  Fixed here.
 */
#define PCI_SUBSYSTEM_VENDOR_ID	0x2c	/* 16 bits was wrongly 0x2e */
#define PCI_SUBSYSTEM_ID	0x2e	/* 16 bits was wrongly 0x2c */

#define PCI_ROM_ADDRESS		0x30	/* 32 bits */
#define PCI_ROM_ADDRESS_ENABLE	0x01	/* Write 1 to enable ROM;
					   bits 31..11 are address,
					   10..2 are reserved */
#define PCI_CAP_PTR		0x34	/* 8 bits: capabilities pointer */
/* 0x35-0x3b are reserved */
#define PCI_INTERRUPT_LINE	0x3c	/* 8 bits */
#define PCI_INTERRUPT_PIN	0x3d	/* 8 bits */
#define PCI_MIN_GNT		0x3e	/* 8 bits */
#define PCI_MAX_LAT		0x3f	/* 8 bits */

/* Power Management registers */
#define PCI_PM_PMC		2
#define PCI_PM_CAP_VER_MASK	0x0007
#define PCI_PM_CAP_PME_CLOCK	0x0008
#define PCI_PM_CAP_RESERVED	0x0010
#define PCI_PM_CAP_DSI		0x0020
#define PCI_PM_CAP_AUX_POWER	0x01c0
#define PCI_PM_CAP_D1		0x0200
#define PCI_PM_CAP_D2		0x0400
#define PCI_PM_CAP_PME		0x0800
#define PCI_PM_CAP_PME_MASK	0xf800
#define PCI_PM_CAP_PME_D0	0x0800
#define PCI_PM_CAP_PME_D1	0x1000
#define PCI_PM_CAP_PME_D2	0x2000
#define PCI_PM_CAP_PME_D3	0x4000
#define PCI_PM_CAP_PME_D3cold	0x8000
#define PCI_PM_CAP_PME_SHIFT	11
#define PCI_PM_CTRL		4
#define PCI_PM_CTRL_STATE_MASK		0x0003
#define PCI_PM_CTRL_NO_SOFT_RESET	0x0008
#define PCI_PM_CTRL_PME_ENABLE		0x0100
#define PCI_PM_CTRL_DATA_SEL_MASK	0x1e00
#define PCI_PM_CTRL_DATA_SCALE_MASK	0x6000
#define PCI_PM_CTRL_PME_STATUS		0x8000
#define PCI_PM_PPB_EXTENSIONS		6
#define PCI_PM_PPB_B2_B3		0x40
#define PCI_PM_BPCC_ENABLE		0x80
#define PCI_PM_DATA_REGISTER		7
#define PCI_PM_SIZEOF			8

/*
 * Resource slot numbering.
 *
 * Bug fixed: the original defined DEVICE_COUNT_RESOURCE both as the last
 * value of this enum (10, from the arithmetic) AND as a plain #define with
 * value 6.  The #define silently won everywhere, making the enum values
 * beyond 5 unreachable and any resource[] array only 6 elements wide while
 * code used enum indices that went higher.
 *
 * Resolution: remove the conflicting #define and size the enum so
 * DEVICE_COUNT_RESOURCE == 6, matching the six standard BARs that this
 * driver actually manages.  PCI_BRIDGE_RESOURCES and PCI_NUM_RESOURCES are
 * kept for source compatibility but adjusted to fit.
 */
#define PCI_BRIDGE_RESOURCES_NUM	4

enum {
	PCI_STD_RESOURCES    = 0,
	PCI_STD_RESOURCES_END = 5,		/* BARs 0..5 */
	PCI_ROM_RESOURCE     = 5,		/* reuse slot 5; ROM not a BAR */
	PCI_BRIDGE_RESOURCES,
	PCI_BRIDGE_RESOURCES_END=PCI_BRIDGE_RESOURCES+PCI_BRIDGE_RESOURCES_NUM-1,
	DEVICE_COUNT_RESOURCE = 6		/* total slots in resource[] */
};

enum {
	PIRQ_GET,
	PIRQ_SET
};

struct resource {
	char		*name;
	u32_t		start;
	u32_t		end;
	u32_t		flags;
	struct resource	*parent, *sibling, *child;
};
typedef struct resource resource_t;

typedef int pci_power_t;

/*
 * One pci_dev per slot-number/function-number combination.
 */
struct pci_dev {
	struct pci_bus	*bus;		/* bus this device is on */
	struct pci_dev	*sibling;	/* next device on this bus */
	struct pci_dev	*next;		/* chain of all devices */

	void		*sysdata;	/* hook for sys-specific data */
	void		*dev;		/* hook for driver-specific data */
	struct pci_dev_info *info;
	int		(*route)(int, int, int);

	u8_t		bn;
	u8_t		devfn;
	u16_t		vendor;
	u16_t		device;
	u16_t		subsystem_vendor;
	u16_t		subsystem_device;
	u32_t		class;		/* 3 bytes: (base, sub, prog-if) */
	u8_t		revision;
	u8_t		hdr_type;
	u32_t		master:1;	/* set if device is master capable */
	u32_t		dma_mask;
	int		is_busmaster:1;
	pci_power_t	current_state;
	u8_t		pm_cap;
	u8_t		irq;		/* IRQ generated by this device */
	u8_t		intpin;
	resource_t	resource[DEVICE_COUNT_RESOURCE];
	int		enable_cnt;
};
typedef struct pci_dev pcidev_t;

struct pci_bus {
	struct list_head  bus_list;
	struct pci_bus	 *parent;	/* parent bus this bridge is on */
	struct pci_bus	 *children;	/* chain of P2P bridges on this bus */
	struct pci_bus	 *next;		/* chain of all PCI buses */

	struct pci_dev	 *self;		/* bridge device as seen by parent */
	struct pci_dev	 *devices;	/* devices behind this bridge */

	void		 *sysdata;	/* hook for sys-specific extension */

	u8_t		  number;	/* bus number */
	u8_t		  primary;	/* number of primary bridge */
	u8_t		  secondary;	/* number of secondary bridge */
	u8_t		  subordinate;	/* max number of subordinate buses */
};
typedef struct pci_bus pcibus_t;

/*
 * Maps a vendor-id/device-id pair to device-specific information.
 */
struct pci_dev_info {
	u16_t	vendor;		/* vendor id */
	u16_t	device;		/* device id */
	char	*name;		/* device name */
	u8_t	bridge_type;	/* bridge type or 0xff */
};
typedef struct pci_dev_info pcidevinfo_t;

struct pci_device_id {
	u16_t	vendor;
	u16_t	device;
	u16_t	subvendor;
	u16_t	subdevice;
	u16_t	class;
	u16_t	class_mask;
	void	*driver_data;
};
typedef struct pci_device_id pcidevid_t;

struct pci_driver {
	char		*name;
	int		(*probe)(pcidev_t *, pcidevid_t *);
	void		(*remove)(pcidev_t *);
	pcidevid_t	*id_table;
	int		*driver;
};

extern pcibus_t	  pci_root;	 /* root bus */
extern pcidev_t	 *pci_devices;	 /* list of all devices */
extern int	  pci_debug;


/*
 * Chipset register shadow structures.
 * These are software copies of selected PCI config registers, filled in
 * by the chipset probe functions.  They are NOT mapped to the hardware
 * address space do not use #pragma pack or rely on field offsets.
 * The offset comments show where each field lives in PCI config space.
 */

typedef struct {
	struct {
		u16_t	vid;		/* 0x00 */
		u16_t	did;		/* 0x02 */
		u16_t	pcicmd;		/* 0x04 */
		u16_t	pcists;		/* 0x06 */
		u8_t	mlt;		/* 0x0d */
		u8_t	hdr;		/* 0x0e */
		u8_t	bist;		/* 0x0f */
		u8_t	intln;		/* 0x3c */
		u8_t	intpn;		/* 0x3d */
		u16_t	pmccfg;		/* 0x50 */
		u8_t	deturbo;	/* 0x52 */
		u8_t	dbc;		/* 0x53 */
		u8_t	axc;		/* 0x54 */
		u16_t	dramr;		/* 0x55 */
		u8_t	dramc;		/* 0x57 */
		u8_t	dramt;		/* 0x58 */
		u8_t	pam[7];		/* 0x59-0x5f */
		u8_t	drb[8];		/* 0x60-0x67 */
		u8_t	fdhc;		/* 0x68 */
		u8_t	mtt;		/* 0x70 */
		u8_t	clt;		/* 0x71 */
		u8_t	smram;		/* 0x72 */
		u8_t	errcmd;		/* 0x90 */
		u8_t	errsts;		/* 0x91 */
		u8_t	trc;		/* 0x93 */
		u8_t	pirqrc[4];	/* 0x60-0x63 (PIRQ route ctrl) */
	} f0;
} i440fx_t;

typedef struct {
	struct {
		u16_t	vid;		/* 0x00 */
		u16_t	did;		/* 0x02 */
		u16_t	pcicmd;		/* 0x04 */
		u16_t	pcists;		/* 0x06 */
		u8_t	hdr;		/* 0x0e */
		u8_t	intln;		/* 0x3c */
		u8_t	intpn;		/* 0x3d */
		u8_t	iort;		/* 0x4c */
		u16_t	xbcs;		/* 0x4e */
		u8_t	pirqrc[4];	/* 0x60-0x63 */
		u8_t	tom;		/* 0x69 */
		u16_t	mstat;		/* 0x6a */  /* was labelled 0x6b - fixed */
		u8_t	mbirq[2];	/* 0x70-0x71 */
		u8_t	mbdma[2];	/* 0x76-0x77 */
		u16_t	pcsc;		/* 0x78 */
		u8_t	apicbase;	/* 0x80 */
		u8_t	dlc;		/* 0x82 */
		u8_t	smicntl;	/* 0xa0 */
		u16_t	smien;		/* 0xa2 */
		u32_t	see;		/* 0xa4 */
		u8_t	ftmr;		/* 0xa8 */
		u16_t	smireq;		/* 0xaa */
		u8_t	ctltmr;		/* 0xac */
		u8_t	cthtmr;		/* 0xad */  /* was wrongly labelled 0xac */
	} f0;
} piix3_t;

typedef struct {
	struct {
		u16_t	vid;		/* 0x00 */
		u16_t	did;		/* 0x02 */
		u16_t	pcicmd;		/* 0x04 */
		u16_t	pcists;		/* 0x06 */
		u8_t	hdr;		/* 0x0e */
		u8_t	intln;		/* 0x3c */
		u8_t	intpn;		/* 0x3d */
		u32_t	pmba;		/* 0x40 */
		u32_t	cnta;		/* 0x44 */
		u32_t	cntb;		/* 0x48 */
		u32_t	gpictl;		/* 0x4c */
		u32_t	devresd;	/* 0x50 */  /* was wrongly after devresa */
		u32_t	devacta;	/* 0x54 */
		u32_t	devactb;	/* 0x58 */
		u32_t	devresa;	/* 0x5c */
		u32_t	devresb;	/* 0x60 */
		u32_t	devresc;	/* 0x64 */
		u32_t	devrese;	/* 0x68 */
		u32_t	devresf;	/* 0x6c */
		u32_t	devresg;	/* 0x70 */
		u32_t	devresh;	/* 0x74 */
		u32_t	devresi;	/* 0x78 */
		u32_t	devresj;	/* 0x7c */
		u32_t	pmregmisc;	/* 0x80 */
		u32_t	smbba;		/* 0x90 */
		u8_t	smbhstcfg;	/* 0xd2 */
		u8_t	smbslvc;	/* 0xd3 */
		u8_t	smbshdw1;	/* 0xd4 */
		u8_t	smbshdw2;	/* 0xd5 */
		u8_t	smbrev;		/* 0xd6 */
		u8_t	pirqrc[4];	/* 0x60-0x63 */
	} f0;
} piix4_t;

typedef struct {
	struct {
		u16_t	vid;		/* 0x00 */
		u16_t	did;		/* 0x02 */
		u16_t	pcicmd;		/* 0x04 */
		u16_t	pcists;		/* 0x06 */
		u8_t	revid;		/* 0x08 */
		u8_t	hdr;		/* 0x0e */
		u8_t	intln;		/* 0x3c */
		u8_t	intpn;		/* 0x3d */
		u8_t	isabuscontrol;	/* 0x40 */
		u8_t	misccontrol1;	/* 0x46 */
		u8_t	misccontrol2;	/* 0x47 */
		u8_t	misccontrol3;	/* 0x48 */
		u8_t	ideirqroute;	/* 0x4a */
		u8_t	pciedgelevel;	/* 0x54 */
		u8_t	pciinta;	/* 0x55 */
		u8_t	pciintbc;	/* 0x56 */
		u8_t	pciintd;	/* 0x57 */
		u8_t	apicirq;	/* 0x58 */
		u8_t	extfunc;	/* 0x85 */
		u8_t	pirqrc[4];	/* vendor-specific */
	} f0;
} via_t;

/* ELCR (Edge/Level Control Register) ports */
#define ELCR1	0x4d0		/* IRQs 0-7  */
#define ELCR2	0x4d1		/* IRQs 8-15 */
#define XBCS	0x04e

/*
 * PCI_SLOT / PCI_FUNC / DEVFN: parenthesise arguments to prevent
 * precedence surprises when callers pass expressions.
 */
#define PCI_SLOT(X)	(((X) >> 3) & 0x1f)
#define PCI_FUNC(X)	((X) & 0x7)
#define DEVFN(S, F)	(((S) << 3) | ((F) & 0x7))

/*
 * PCI config space accessors.
 */
#define prcb(b,d,r,p)	pci(PCI_RD,(b),(d),(r),(u32_t)(ptrsize_t)(p),sizeof(u8_t))
#define prcw(b,d,r,p)	pci(PCI_RD,(b),(d),(r),(u32_t)(ptrsize_t)(p),sizeof(u16_t))
#define prcd(b,d,r,p)	pci(PCI_RD,(b),(d),(r),(u32_t)(ptrsize_t)(p),sizeof(u32_t))

#define pwcb(b,d,r,v)	pci(PCI_WR, (b), (d), (r), (u32_t)(v), sizeof(u8_t))
#define pwcw(b,d,r,v)	pci(PCI_WR, (b), (d), (r), (u32_t)(v), sizeof(u16_t))
#define pwcd(b,d,r,v)	pci(PCI_WR, (b), (d), (r), (u32_t)(v), sizeof(u32_t))

enum intr_trigger {
	INTR_TRIGGER_INVALID	= -1,
	INTR_TRIGGER_CONFORM	= 0,
	INTR_TRIGGER_EDGE	= 1,
	INTR_TRIGGER_LEVEL	= 2
};

enum intr_polarity {
	INTR_POLARITY_CONFORM	= 0,
	INTR_POLARITY_HIGH	= 1,
	INTR_POLARITY_LOW	= 2
};

typedef struct {
	u8_t	pl_id;		/* link register value */
	u8_t	pl_irq;		/* assigned IRQ, PCI_INVALID_IRQ if none */
	u16_t	pl_irqmask;	/* bitmap of IRQs this link can use */
	int	pl_references;	/* number of INTx pins sharing this link */
	int	pl_routed;	/* non-zero if BIOS has already routed */
	List_t	list;
} pci_link_t;

typedef struct {
	pci_link_t **pci_link_ptr;
	u8_t	bn;
	u8_t	devfn;
	int	pin;
} pci_link_lookup_t;

#include "pci_bios.h"

typedef void pir_entry_handler(PIR_entry_t *,PIR_intpin_t *, void *);
#include "pci_funcs.h"

extern int pir_router_found;
extern int verbose_bios;

#endif /* PCI_H */
