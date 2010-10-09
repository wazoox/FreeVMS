#include "freevms.h"

int
main(void)
{
	L4_Fpage_t					kip_area;
	L4_Fpage_t					utcb_area;

	L4_KernelInterfacePage_t	*kip;

	L4_ThreadId_t				roottid;
	L4_ThreadId_t				s0tid;

	L4_Word_t					page_bits;
	L4_Word_t					utcb_size;

	printf("\n");
	printf(">>> Starting FreeVMS kernel initialization.\n");
	printf("\n");

	kip = (L4_KernelInterfacePage_t *) L4_KernelInterface();
	roottid = L4_Myself();
	s0tid = L4_GlobalId(kip->ThreadInfo.X.UserBase, 1);

	printf("%%KERN$CPUS_NUMBER = %d\n", kip->ProcessorInfo.X.processors + 1);
	for(page_bits = 0;
			!((1 << page_bits) & L4_PageSizeMask(kip)); page_bits++);
	printf("%%KERN$PAGE_SIZE   = $%016lX\n", (1 << page_bits));

	kip_area = L4_FpageLog2((L4_Word_t) kip, L4_KipAreaSizeLog2 (kip));
	printf("%%KERN$KIP_AREA    = $%016lX\n", kip_area);
	//utcb_area = L4_FpageLog2

	printf("\n");
	printf(">>> System is coming up.\n");
	printf("\n");

	while(1);

	printf(">>> System halted\n");
	return(0);
}
