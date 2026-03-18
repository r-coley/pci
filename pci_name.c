/*
 * pci_name.c
 */
#include <sys/types.h>

#include "pci.h"
#include "pci_funcs.h"

static pci_class_t class_list[] =
{
	{ 0x00, "Unclassified device",
	  {
	    { 0x00, "Non-VGA device" },
	    { 0x01, "VGA compatible device" },
	    { 0x00, 0 },
	  }
	},
	{ 0x01, "Mass storage",
	  {
	    { 0x00, "SCSI storage controller" },
	    { 0x01, "IDE controller" },
	    { 0x02, "Floppy disk controller" },
	    { 0x03, "IPI bus controller" },
	    { 0x04, "RAID bus controller" },
	    { 0x05, "ATA controller" },
	    { 0x06, "SATA controller" },
	    { 0x07, "Serial Attached SCSI controller" },
	    { 0x08, "Non-Volatile memory controller" },
	    { 0x80, "Mass storage controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x02, "Network",
	  {
	    { 0x00, "Ethernet controller" },
	    { 0x01, "Token ring network controller" },
	    { 0x02, "FDDI network controller" },
	    { 0x03, "ATM network controller" },
	    { 0x04, "ISDN controller" },
	    { 0x05, "WorldFip controller" },
	    { 0x06, "PICMG controller" },
	    { 0x80, "Network controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x03, "Display",
	  {
	    { 0x00, "VGA compatible controller" },
	    { 0x01, "XGA compatible controller" },
	    { 0x02, "3D controller" },
	    { 0x80, "Display controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x04, "Multimedia",
	  {
	    { 0x00, "Multimedia video controller" },
	    { 0x01, "Multimedia audio controller" },
	    { 0x02, "Computer telephony device" },
	    { 0x03, "Audio device" },
	    { 0x80, "Multimedia controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x05, "Memory",
	  {
	    { 0x00, "RAM memory" },
	    { 0x01, "FLASH memory" },
	    { 0x80, "Memory controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x06, "Bridge",
	  {
	    { 0x00, "Host bridge" },
	    { 0x01, "ISA bridge" },
	    { 0x02, "EISA bridge" },
	    { 0x03, "MicroChannel bridge" },
	    { 0x04, "PCI bridge" },
	    { 0x05, "PCMCIA bridge" },
	    { 0x06, "NuBus bridge" },
	    { 0x07, "CardBus bridge" },
	    { 0x08, "RACEway bridge" },
	    { 0x09, "Semi-transparent PCI-to-PCI bridge" },
	    { 0x0a, "InfiniBand to PCI host bridge" },
	    { 0x80, "Bridge" },
	    { 0x00, 0 },
	  }
	},
	{ 0x07, "Communication",
	  {
	    { 0x00, "Serial controller" },
	    { 0x01, "Parallel controller" },
	    { 0x02, "Multiport serial controller" },
	    { 0x03, "Modem" },
	    { 0x04, "GPIB controller" },
	    { 0x05, "Smart Card controller" },
	    { 0x80, "Communication controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x08, "Generic system peripheral",
	  {
	    { 0x00, "PIC" },
	    { 0x01, "DMA controller" },
	    { 0x02, "Timer" },
	    { 0x03, "RTC" },
	    { 0x04, "PCI Hot-plug controller" },
	    { 0x05, "SD Host controller" },
	    { 0x06, "IOMMU" },
	    { 0x80, "System peripheral" },
	    { 0x00, 0 },
	  }
	},
	/*
	 * Class 0x09: Input device controllers.
	 */
	{ 0x09, "Input device",
	  {
	    { 0x00, "Keyboard controller" },
	    { 0x01, "Digitiser" },
	    { 0x02, "Mouse controller" },
	    { 0x03, "Scanner controller" },
	    { 0x04, "Gameport controller" },
	    { 0x80, "Input device controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x0a, "Docking station",
	  {
	    { 0x00, "Generic Docking Station" },
	    { 0x80, "Docking Station" },
	    { 0x00, 0 },
	  }
	},
	{ 0x0b, "Processor",
	  {
	    { 0x00, "386" },
	    { 0x01, "486" },
	    { 0x02, "Pentium" },
	    { 0x10, "Alpha" },
	    { 0x20, "Power PC" },
	    { 0x30, "MIPS" },
	    { 0x40, "Co-processor" },
	    { 0x00, 0 },
	  }
	},
	{ 0x0c, "Serial bus",
	  {
	    { 0x00, "FireWire (IEEE 1394)" },
	    { 0x01, "ACCESS Bus" },
	    { 0x02, "SSA" },
	    { 0x03, "USB controller" },
	    { 0x04, "Fibre Channel" },
	    { 0x05, "SMBus" },
	    { 0x06, "InfiniBand" },
	    { 0x07, "IPMI SMIC interface" },
	    { 0x08, "SERCOS interface" },
	    { 0x09, "CANBUS" },
	    { 0x00, 0 },
	  }
	},
	/*
	 * Class 0x0d: Wireless controllers.
	 */
	{ 0x0d, "Wireless",
	  {
	    { 0x00, "IRDA controller" },
	    { 0x01, "Consumer IR controller" },
	    { 0x10, "RF controller" },
	    { 0x11, "Bluetooth" },
	    { 0x12, "Broadband" },
	    { 0x20, "802.1a controller" },
	    { 0x21, "802.1b controller" },
	    { 0x80, "Wireless controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x0e, "Intelligent controller",
	  {
	    { 0x00, "I2O" },
	    { 0x00, 0 },
	  }
	},
	{ 0x0f, "Satellite communications",
	  {
	    { 0x01, "Satellite TV controller" },
	    { 0x02, "Satellite audio communication controller" },
	    { 0x03, "Satellite voice communication controller" },
	    { 0x04, "Satellite data communication controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x10, "Encryption",
	  {
	    { 0x00, "Network and computing encryption device" },
	    { 0x10, "Entertainment encryption device" },
	    { 0x80, "Encryption controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0x11, "Signal processing",
	  {
	    { 0x00, "DPIO module" },
	    { 0x01, "Performance counters" },
	    { 0x10, "Communication synchronizer" },
	    { 0x20, "Signal processing management" },
	    { 0x80, "Signal processing controller" },
	    { 0x00, 0 },
	  }
	},
	{ 0xff, "Unassigned class",
	  {
	    { 0x00, 0 }
	  }
	},
	{ 0x00, 0,
	  {
	    { 0x00, 0 }
	  }
	}
};

/*
 * Device/vendor name lookup now comes from successful
 * pci_register_driver() matches.  Keep only class/subclass
 * naming in the PCI core.
 */

char *
get_pci_vendor_name(u16_t vendor)
{
	return 0;
}

char *
get_pci_class_name(u8_t class)
{
	int i;

	for (i = 0; class_list[i].class_name; i++)
		if (class_list[i].class_id == class) break;
	return class_list[i].class_name;
}

/*
 * get_pci_subclass_name() return the subclass name for a given
 * (class, subclass) pair.
 *
 * The 'subclass' parameter carries only the 8-bit subclass byte
 * (bits 15..8 of the PCI class register); the caller passes it as
 * a u16_t but only the low byte is meaningful here.  The hsclass/
 * lsclass split was a leftover from an earlier design and is now
 * unused the comparison is a straight byte match on lsclass.
 *
 * Bug fixed: the secondary match condition
 *   if (x&0xff == 0 && x>>8 == hsclass) break;
 * was missing parentheses around the &0xff operand, so it parsed as
 *   if (x & (0xff == 0) && ...) i.e. if (x & 0) which is always false,
 * meaning the secondary path never fired.  Since subclass_id is a u8_t
 * and subclass is now used correctly as an 8-bit value, both conditions
 * reduce to a single direct comparison and the secondary path is removed.
 */
char *
get_pci_subclass_name(u8_t class, u8_t subclass)
{
	int	i, j;

	for (i = 0; class_list[i].class_name; i++)
		if (class_list[i].class_id == class) break;

	if (!class_list[i].class_name)
		return "???";

	for (j = 0; class_list[i].subclass_list[j].subclass_name; j++)
		if (class_list[i].subclass_list[j].subclass_id == subclass)
			break;

	if (!class_list[i].subclass_list[j].subclass_name)
		return "???";
	return class_list[i].subclass_list[j].subclass_name;
}

char *
get_pci_dev(u16_t vendor, u16_t device)
{
	return 0;
}
