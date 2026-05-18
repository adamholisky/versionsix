Physical				 Virtual				  Description
0x0000 0000 0010 0000    0xffff 8000 0010 0000    Kernel usable memory (size: 0xbfa1 7000 as of 5/15/26)
0x0000 0000 bfb2 6000    0xffff ffff 8000 0000    Kernel base/start
0x0000 0000 bfce 6000    0xffff ffff 801c 0000    Kernel end (as of 5/15/26)
0x0000 0000 fd00 0000    0xffff 8000 fd00 0000    Framebuffer
0x0000 0000 0000 0000    0xffff 8000 0000 0000    Identity map start
0x0000 0002 4000 0000    0x0000 7001 4000 0000	  Kernel heap end (5 GiB)
0x0000 0001 0000 0000    0x0000 7000 0000 0000    Kernel heap start
0x---- ---- ---- ----    0x0000 1000 0000 0000    Shared libraries start
0x---- ---- ---- ----    0x0000 0000 B000 0000    Process heap start
0x---- ---- ---- ----    0x0000 0000 A001 0000    Process stack end (16 pages, 65,535 bytes)
0x---- ---- ---- ----    0x0000 0000 A000 0000    Process stack start
0x---- ---- ---- ----    0x0000 0000 0800 0000    Process program load start
						 