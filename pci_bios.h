/*
 * pci_bios.h
 */
#ifndef PCI_BIOS_H
#define PCI_BIOS_H

#define BIOS_START	0xe0000
#define BIOS_SIZE	0x20000

#define BIOS_PADDRTOVADDR(addr)		phystokv(addr)
#define BIOS_VADDRTOPADDR(addr) 	kvtophys(addr)
#define ADDR2EPS(addr)	((struct smbios_eps *)(addr))

#define SMBIOS_START	0xf0000
#define SMBIOS_SIG	"_SM_"
#define SMBIOS_LEN	4
#define SMBIOS_STEP	0x10
#define SMBIOS_OFF	0x0

#define SMBIOS3_START	0xf0000
#define SMBIOS3_SIG	"_SM3_"
#define SMBIOS3_LEN	5
#define SMBIOS3_STEP	0x10
#define SMBIOS3_OFF	0x0

#pragma pack(1)
struct smbios_eps {
	u8_t	anchor_string[4];
	u8_t	checksum;
	u8_t	length;
	u8_t	major_version;	
	u8_t 	minor_version;	
	u16_t	maximum_structure_size;
	u8_t	entry_point_revision;
	u8_t	formatted_area[5];
	u8_t	intermediate_anchor_string[5];
	u8_t	intermediate_checksum;
	u16_t	structure_table_length;
	u32_t	structure_table_address;
	u16_t	number_structures;
	u8_t	BCD_revision;
};

#pragma pack(1)
struct smbios_structure_header {
	u8_t	type;
	u8_t	length;
	u16_t	handle;
};

#define VPD_START	0xf0000
#define VPD_SIG		"VPD"
#define VPD_LEN		3
#define VPD_STEP	0x10
#define VPD_OFF		0x2

#define PIR_START	0x0
#define PIR_SIG		"$PIR"
#define PIR_LEN		4
#define PIR_STEP	0x10
#define PIR_OFF		0x0

#define BIOS32_START	0x0
#define BIOS32_SIG	"_32_"
#define BIOS32_LEN	4
#define BIOS32_STEP	0x10
#define BIOS32_OFF	0x0

#define PNP_START	0x0
#define PNP_SIG		"$PnP"
#define PNP_LEN		4
#define PNP_STEP	0x10
#define PNP_OFF		0x0

#define SMAPI_START	0x0
#define SMAPI_SIG	"$SMB"
#define SMAPI_LEN	4
#define SMAPI_STEP	0x10
#define SMAPI_OFF	0x0

#define PSB_START	0x0
#define PSB_SIG		"AMDK7PNOW"
#define PSB_LEN		10
#define PSB_STEP	0x10
#define PSB_OFF		0x0

#define DMI_START	0x0
#define DMI_SIG		"_DMI_"
#define DMI_LEN		5
#define DMI_STEP	0x10
#define DMI_OFF		0x0

#define SYSID_START	0x0
#define SYSID_SIG	"_SYSID_"
#define SYSID_LEN	7
#define SYSID_STEP	0x10
#define SYSID_OFF	0x0

#define MP_START	0x0
#define MP_SIG		"_MP_"
#define MP_LEN		4
#define MP_STEP		0x10
#define MP_OFF		0x0

#define ACPI_START	0x0
#define ACPI_SIG	"RSD PTR "
#define ACPI_LEN	8
#define ACPI_STEP	0x10
#define ACPI_OFF	0x0

#define SONY_START	0x0
#define SONY_SIG	"$SNY"
#define SONY_LEN	4
#define SONY_STEP	0x10
#define SONY_OFF	0x0

#pragma pack(1)
typedef struct {
	char	Signature[8];
	u8_t	Checksum;
	char	OEMID[6];
	u8_t	Revision;
	u32_t	rsdtAddress;
} RSDP_t; 

#pragma pack(1)
typedef struct {
	char	Signature[8];
	u8_t	Checksum;
	char	OEMID[6];
	u8_t	Revision;
	u32_t	rsdtAddress;

	/* RSDP v2.0 */
	u32_t	length;
	u64_t	xsdtAddress;
	u8_t	checksumExt;
	u8_t	reserved[3];
} XSDP_t;

#pragma pack(1)
typedef struct {
	char 	Signature[4];
	u32_t	Length;
	u8_t	Revision;
	u8_t	Checksum;
	char	OEMID[6];
	char	OEMTableID[8];
	u32_t	OEMRevision;
	u32_t	CreatorID;
	u32_t	CreatorRev;
} ACPI_SDT_Hdr_t;

typedef struct {
	ACPI_SDT_Hdr_t hdr;
	u32_t *PtrToOtherSDT;
} RSDT_t;

#endif /* PCI_BIOS_H */
