 // JdS 2004/09/26 {
 struct VESA_0 {
		int  mode_atr;     // Mode atributy  !!! Pro VESA standart
		char win_A;        // Window A atr
		char win_B;        // Window B atr
		int  granul;       // granularity
		int  win_size;     // window size
		int  seg_adr_A;    // Segment adr A
		int  seg_adr_B;    // Segment adr B
		char far *adrwin;  // Address for windows call;
		char bit_planes;   // Pocet bit. planes
		char bit_pixel;    // Bitu na pixel
		char mem_banks;    // Number mem. banks
		char mem_model;    // Memory model
		int bytes_lin;	    // Bytes/line
		char rezerva[2];   // 2B rezerva     SUM = 24Bytu
    };
 // JdS 2004/09/26 }

 struct VESA_1 {
		char  vesa[4];     // "VESA"
		char  vnum1,vnum2; // Vesa number
		char  far *manuf;  // Manuf. string
		long  capa;        // Zatim nic
		int   far *mode;   // Tabulka modu
		int   mem64;       // Pocet 64k seg. videoram
		char  fill[240];
    };

 struct VESA_2 {
		int  mode_atr;     // Mode atributy
		char win_A;        // Window A atr
		char win_B;        // Window B atr
		int  granul;       // granularity
		int  win_size;     // window size
		int  seg_adr_A;    // Segment adr A
		int  seg_adr_B;    // Segment adr B
		char far *adrwin;  // Address for windows call;
		int  bytes_lin;    // Bytu na scan line
		//------------------ Dodatecne informace (bit 1 mode_atr)
		int  horiz_res;    // Horizontal resolution
		int  vert_res;     // Vertical  resolution
		char ch_width;     // Character width
		char ch_height;    // Character height
		char bit_planes;   // Pocet bit. planes
		char bit_pixel;    // Bitu na pixel
		char mem_banks;    // Number mem. banks
		char mem_model;    // Memory model
		char siz_banks;    // Size of mem. bank in kB
		char displ_pag;    // Display pages
		char reserved;
		char fill[230];
    };

   //-----------------------------------------------
