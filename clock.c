void SetSysClockTo400mHz (void)
{
	volatile unsigned long reg, value;

	// *********************************************************************
	// update voltage regulator configuration
	// *********************************************************************
	PWR->CR3 &= ~PWR_CR3_SCUEN; // lock configuration
	while ((PWR->CSR1 & PWR_CSR1_ACTVOSRDY) == 0) {}; // wait for voltage

	// *********************************************************************
	// Set voltage scaling to VOS1 (to reach 400 MHz)
	// *********************************************************************
	PWR->D3CR |= (3UL << 14); // set VOS1
	while((PWR->D3CR & PWR_D3CR_VOSRDY) == 0) {}; // wait for voltage
		
	// *********************************************************************
	// Turning on external quartz and waiting for readiness.
	// *********************************************************************
	
	// Enable HSE
	RCC->CR |= RCC_CR_HSEON;
	
	// Wait till HSE is ready
	while((RCC->CR & RCC_CR_HSERDY) == 0) {};
		
	// *********************************************************************
	// Specify the frequency source for PLL1 - external quartz.
	// *********************************************************************
	RCC->PLLCKSELR |= RCC_PLLCKSELR_PLLSRC_HSE;
	
	// *********************************************************************
	// The value of the divider is set to 4.
	// *********************************************************************	
		
	// The value of the divider is set to 4.
		
	// 000000: prescaler disabled
	// 000001: division by 1 (bypass)
	// 000010: division by 2
	// 000011: division by 3
	// 000100: division by 4
	// ...
	// 100000: division by 32 (default after reset)
	// ...
	// 111111: division by 63

	reg  = RCC->PLLCKSELR;                     // read
	reg &= ~RCC_PLLCKSELR_DIVM1;               // clear
	reg |= (DIVM1 << RCC_PLLCKSELR_DIVM1_Pos); // modify
	RCC->PLLCKSELR = reg;                      // set

	// *********************************************************************	
	// The multiplier N and dividers P, Q, R for PLL1DIVR
	// *********************************************************************	
		
	// DIVN1[8:0]  0  - 8   PLLN = 400
	// DIVP1[6:0]  9  - 15  PLLP = 2
	// DIVQ1[6:0]  16 - 22  PLLQ = 2
	// DIVR1[6:0]  24 - 30  PLLR = 2
	
	reg = RCC->PLL1DIVR; // read
	
	reg &= ~RCC_PLL1DIVR_N1; // clear 
	reg &= ~RCC_PLL1DIVR_P1; // clear
	reg &= ~RCC_PLL1DIVR_Q1; // clear
	reg &= ~RCC_PLL1DIVR_R1; // clear
	
	if      (DIVN1 <=   4) value = 3;
	else if (DIVN1 >= 512) value = 511;
	else                   value = DIVN1 - 1;
	reg |= (value << RCC_PLL1DIVR_N1_Pos); // modify
	
	if      (DIVP1 <=   1) value = 0;
	else if (DIVP1 >= 128) value = 127;
	else                   value = DIVP1 - 1;
	reg |= (value << RCC_PLL1DIVR_P1_Pos); // modify
	
	if      (DIVQ1 <=   1) value = 0;
	else if (DIVQ1 >= 128) value = 127;
	else                   value = DIVQ1 - 1;
	reg |= (value << RCC_PLL1DIVR_Q1_Pos); // modify
	
	if      (DIVR1 <=   1) value = 0;
	else if (DIVR1 >= 128) value = 127;
	else                   value = DIVR1 - 1;
	reg |= (value << RCC_PLL1DIVR_R1_Pos); // modify
	
	RCC->PLL1DIVR = reg; // set
	
	// *********************************************************************	
	// Fractional PLL frequency divider (if needed)
	// *********************************************************************	
	
	reg  = RCC->PLL1FRACR;                       // read
	reg &= ~RCC_PLL1FRACR_FRACN1;                // clear
	reg |= (FRACN1 << RCC_PLL1FRACR_FRACN1_Pos); // modify
	RCC->PLL1FRACR = reg;                        // set
	
	// *********************************************************************	
	// [6] Specify the input frequency range PLL1	
	// *********************************************************************	
	
	// 00: The PLL1 input (ref1_ck) clock range frequency is between 1 and 2  MHz (default after reset)
	// 01: The PLL1 input (ref1_ck) clock range frequency is between 2 and 4  MHz
	// 10: The PLL1 input (ref1_ck) clock range frequency is between 4 and 8  MHz
	// 11: The PLL1 input (ref1_ck) clock range frequency is between 8 and 16 MHz
	
	reg  = RCC->PLLCFGR;                          // red
	reg &= ~RCC_PLLCFGR_PLL1RGE;                  // clear
	reg |= (PLL1_RGE << RCC_PLLCFGR_PLL1RGE_Pos); // modify
	RCC->PLLCFGR = reg;                           // set
	
	// *********************************************************************
	// Specify the output frequency range of PLL1
	// *********************************************************************
	
	// 0: Wide   VCO range: 192 to 836 MHz (default after reset)
	// 1: Medium VCO range: 150 to 420 MHz
	
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL1VCOSEL;
	
	// *********************************************************************
	// Enable output dividers PLL1: P, Q, R
	// *********************************************************************
	
	//Bit 16 DIVP1EN: PLL1 DIVP divider output enable
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;
	
    // Enable PLL1Q Clock output
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ1EN;
	
    // Enable PLL1R  Clock output
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVR1EN;
	
	// *********************************************************************
	// The inclusion of a fractional divider.
	// *********************************************************************
	
	// Enable PLL1FRACN
	//RCC->PLLCFGR |= RCC_PLLCFGR_PLL1FRACEN;
	
	// *********************************************************************
	// Start PLL1 and wait for readiness
	// *********************************************************************
	
    RCC->CR |= RCC_CR_PLLON;
    while((RCC->CR & RCC_CR_PLL1RDY) == 0) {};
		
	// PLL1 is configured and running. Now select the source of the SYSCLK frequency and set up the bus dividers.
		
	// *********************************************************************	
	// Divider by 2 HPRE = 0
	// *********************************************************************
		
    // HPRE[3:0]: D1 domain AHB prescaler
		
    // 1000: rcc_hclk3 = sys_d1cpre_ck / 2
	// 1001: rcc_hclk3 = sys_d1cpre_ck / 4
	// 1010: rcc_hclk3 = sys_d1cpre_ck / 8
	// 1011: rcc_hclk3 = sys_d1cpre_ck / 16
	// 1100: rcc_hclk3 = sys_d1cpre_ck / 64
	// 1101: rcc_hclk3 = sys_d1cpre_ck / 128
	// 1110: rcc_hclk3 = sys_d1cpre_ck / 256
	// 1111: rcc_hclk3 = sys_d1cpre_ck / 512
	
	if      (HPRE <= 1  ) value = 0;
	else if (HPRE == 2  ) value = 8;
	else if (HPRE == 4  ) value = 9;
	else if (HPRE == 8  ) value = 10;
	else if (HPRE == 16 ) value = 11;
	else if (HPRE == 64 ) value = 12;
	else if (HPRE == 128) value = 13;
	else if (HPRE == 256) value = 14;
	else if (HPRE >= 512) value = 15;
	else                  value = 15;
		
	reg  = RCC->D1CFGR;                    // read
    reg &= ~RCC_D1CFGR_HPRE;               // clear
	reg |= (value << RCC_D1CFGR_HPRE_Pos); // modify	
	RCC->D1CFGR = reg;                     // set

	// *********************************************************************	
	// Without division D1CPRE = 0b1000
	// *********************************************************************

    // D1CPRE[3:0]: D1 domain Core prescaler
	
    // 0xxx: sys_ck not divided (default after reset)
	// 1000: sys_ck divided by 2
	// 1001: sys_ck divided by 4
	// 1010: sys_ck divided by 8
	// 1011: sys_ck divided by 16
	// 1100: sys_ck divided by 64
	// 1101: sys_ck divided by 128
	// 1110: sys_ck divided by 256
	// 1111: sys_ck divided by 512
	
	if      (D1CPRE <= 1  ) value = 0;
	else if (D1CPRE == 2  ) value = 8;
	else if (D1CPRE == 4  ) value = 9;
	else if (D1CPRE == 8  ) value = 10;
	else if (D1CPRE == 16 ) value = 11;
	else if (D1CPRE == 64 ) value = 12;
	else if (D1CPRE == 128) value = 13;
	else if (D1CPRE == 256) value = 14;
	else if (D1CPRE >= 512) value = 15;
	else                    value = 15;
	
	reg = RCC->D1CFGR;                       // read
    reg &= ~RCC_D1CFGR_D1CPRE;               // clear
	reg |= (value << RCC_D1CFGR_D1CPRE_Pos); // modify
	RCC->D1CFGR = reg;                       // set
	
	// *********************************************************************
	// Set PLL1 as source of SYSCLK and expect readiness.
	// *********************************************************************
	
	// 000: HSI selected as system clock (hsi_ck) (default after reset)
	// 001: CSI selected as system clock (csi_ck)
	// 010: HSE selected as system clock (hse_ck)
	// 011: PLL1 selected as system clock (pll1_p_ck)

	reg  = RCC->CFGR;        // read
	reg &= ~RCC_CFGR_SW;     // clear
	reg |= RCC_CFGR_SW_PLL1; // modify
	RCC->CFGR = reg;         // set
	
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL1) {};

	// *********************************************************************	
	// Divider by 2 D1PPRE = 2
	// *********************************************************************
		
    // Bits 6:4 D1PPRE[2:0]: D1 domain APB3 prescaler
		
	// 0xx: rcc_pclk3 = rcc_hclk3 (default after reset)
	// 100: rcc_pclk3 = rcc_hclk3 / 2
	// 101: rcc_pclk3 = rcc_hclk3 / 4
	// 110: rcc_pclk3 = rcc_hclk3 / 8
	// 111: rcc_pclk3 = rcc_hclk3 / 16
	
	if      (D1PPRE <= 1 ) value = 0;
	else if (D1PPRE == 2 ) value = 4;
	else if (D1PPRE == 4 ) value = 5;
	else if (D1PPRE == 8 ) value = 6;
	else if (D1PPRE >= 16) value = 7;
	else                   value = 7;
		
	reg  = RCC->D1CFGR;                      // read
	reg &= ~RCC_D1CFGR_D1PPRE;               // clear
	reg |= (value << RCC_D1CFGR_D1PPRE_Pos); // modify
	RCC->D1CFGR = reg;                       // set
	

	
	// *********************************************************************		
	// Divider by 2 D2PPRE1 = 2
	// *********************************************************************
		
	// Bits 6:4 D2PPRE1[2:0]: D2 domain APB1 prescaler
		
	// 0xx: rcc_pclk1 = rcc_hclk1 (default after reset)
	// 100: rcc_pclk1 = rcc_hclk1 / 2
	// 101: rcc_pclk1 = rcc_hclk1 / 4
	// 110: rcc_pclk1 = rcc_hclk1 / 8
	// 111: rcc_pclk1 = rcc_hclk1 / 16
	
	if      (D2PPRE1 <= 1 ) value = 0;
	else if (D2PPRE1 == 2 ) value = 4;
	else if (D2PPRE1 == 4 ) value = 5;
	else if (D2PPRE1 == 8 ) value = 6;
	else if (D2PPRE1 >= 16) value = 7;
	else                    value = 7;
	
	reg  = RCC->D2CFGR;                       // read
	reg &= ~RCC_D2CFGR_D2PPRE1;               // clear
	reg |= (value << RCC_D2CFGR_D2PPRE1_Pos); // modify
	RCC->D2CFGR = reg;                        // set
	

	
	// *********************************************************************
	// Divider by 2 D2PPRE2 = 2
	// *********************************************************************

	// Bits 10:8 D2PPRE2[2:0]: D2 domain APB2 prescaler
	
	// 0xx: rcc_pclk2 = rcc_hclk1 (default after reset)
	// 100: rcc_pclk2 = rcc_hclk1 / 2
	// 101: rcc_pclk2 = rcc_hclk1 / 4
	// 110: rcc_pclk2 = rcc_hclk1 / 8
	// 111: rcc_pclk2 = rcc_hclk1 / 16
	
	if      (D2PPRE2 <= 1 ) value = 0;
	else if (D2PPRE2 == 2 ) value = 4;
	else if (D2PPRE2 == 4 ) value = 5;
	else if (D2PPRE2 == 8 ) value = 6;
	else if (D2PPRE2 >= 16) value = 7;
	else                    value = 7;
	
	reg  = RCC->D2CFGR;                       // read
	reg &= ~RCC_D2CFGR_D2PPRE2;               // clear
	reg |= (value << RCC_D2CFGR_D2PPRE2_Pos); // modify
	RCC->D2CFGR = reg;                        // set
	

	
	// *********************************************************************
	// Divider by 2 D3PPRE = 2
	// *********************************************************************
	
	// Bits 6:4 D3PPRE[2:0]: D3 domain APB4 prescaler
	
	// 0xx: rcc_pclk4 = rcc_hclk4 (default after reset)
	// 100: rcc_pclk4 = rcc_hclk4 / 2
	// 101: rcc_pclk4 = rcc_hclk4 / 4
	// 110: rcc_pclk4 = rcc_hclk4 / 8
	// 111: rcc_pclk4 = rcc_hclk4 / 16
	
	if      (D3PPRE <= 1 ) value = 0;
	else if (D3PPRE == 2 ) value = 4;
	else if (D3PPRE == 4 ) value = 5;
	else if (D3PPRE == 8 ) value = 6;
	else if (D3PPRE >= 16) value = 7;
	else                   value = 7;
	
	reg  = RCC->D3CFGR;                      // read
	reg &= ~RCC_D3CFGR_D3PPRE;               // clear
	reg |= (value << RCC_D3CFGR_D3PPRE_Pos); // modify
	RCC->D3CFGR = reg;                       // set
}
