/*
 * M200 pll configuration
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define DEBUG
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

struct pll_cfg {
	unsigned apll_freq;
	unsigned mpll_freq;
	unsigned cdiv;
	unsigned l2div;
	unsigned h0div;
	unsigned h2div;
	unsigned pdiv;
} pll_cfg;

#define SEL_SRC		0X1
#define SEL_CPLL	((CONFIG_CPU_SEL_PLL == APLL) ? 0x1 : 0x2)
#define SEL_H0CLK	((CONFIG_DDR_SEL_PLL == APLL) ? 0x1 : 0x2)
#define SEL_H2CLK	SEL_H0CLK

#ifndef CONFIG_SYS_CPCCR_SEL
#define CPCCR_CFG	\
	(((SEL_SRC& 3) << 30)					   \
	 | ((SEL_CPLL & 3) << 28)				   \
	 | ((SEL_H0CLK & 3) << 26)                 \
	 | ((SEL_H2CLK & 3) << 24)                 \
	 | (((pll_cfg.pdiv- 1) & 0xf) << 16)	   \
	 | (((pll_cfg.h2div - 1) & 0xf) << 12)	   \
	 | (((pll_cfg.h0div - 1) & 0xf) << 8)	   \
	 | (((pll_cfg.l2div - 1) & 0xf) << 4)	   \
	 | (((pll_cfg.cdiv - 1) & 0xf) << 0))
#else
/**
 * Board CPCCR configure.
 * CONFIG_SYS_CPCCR_SEL should be define in [board].h
 */
#define CPCCR_CFG CONFIG_SYS_CPCCR_SEL
#endif

static unsigned int get_pllreg_value(int freq)
{
	cpm_cpxpcr_t cppcr;
	unsigned int pllfreq = freq / 1000000;
	unsigned int extal = gd->arch.gi->extal / 1000000;
	unsigned nr = 0,nf = 0,od1 =7 ,od0;

	/*Unset*/
	if (freq < 600000000) {
		error("uboot pllfreq must greater than 600M");
		return -EINVAL;
	}

	/*Align to extal clk*/
	if (pllfreq%extal  >= extal/2) {
		pllfreq += (extal - pllfreq%extal);
	} else {
		pllfreq -= pllfreq%extal;
	}

	/*caculate nf*/
	do {
		nr++;
		nf = (pllfreq*nr)/extal;
	} while ((nf * extal != nr * pllfreq || nf >= 4096) && nr < 63);

	/*caculate od1*/
	while ((nr%od1) && od1 > 1) {
		od1--;
	}
	nr = nr/od1;

	/*caculate od0*/
	od0 = od1;
	while((nr%od0) && od0 > 1) {
		od0--;
	}
	nr = nr/od0;

	cppcr.b.PLLM = nf;
	cppcr.b.PLLN = nr;
	cppcr.b.PLLOD0 = od0;
	cppcr.b.PLLOD1 = od1;
	printf("nf=%d nr = %d od0 = %d od1 = %d\n",nf,nr,od0,od1);
	printf("cppcr is %x\n",cppcr.d32);

	return cppcr.d32;
}

static void pll_set(int pll,int freq)
{
	unsigned int regvalue = get_pllreg_value(freq);
	cpm_cpxpcr_t cppcr;

	if (regvalue == -EINVAL)
		return;
	switch (pll) {
		case APLL:
			/* Init APLL */
			cpm_outl(regvalue | (0x1 << 0), CPM_CPAPCR);
			while(!(cpm_inl(CPM_CPAPCR) & (0x1 << 3)))
				;
			debug("CPM_CPAPCR %x\n", cpm_inl(CPM_CPAPCR));
			break;
		case MPLL:
			/* Init MPLL */
			cppcr.d32 = regvalue;
			if(freq <= 800000000) {
				cppcr.b.PLLM *= 2;
				cppcr.b.PLLOD1 *= 2;
			} else {

			}
			cpm_outl(cppcr.d32 | (0x1 << 0), CPM_CPMPCR);
			while(!(cpm_inl(CPM_CPMPCR) & (0x1 << 3)))
				;
			debug("CPM_CPMPCR %x\n", cpm_inl(CPM_CPMPCR));
			break;
	}
}

static void cpccr_init(void)
{
	unsigned int cpccr;

	/* change div */
	cpccr = (cpm_inl(CPM_CPCCR) & (0xff << 24))
		| (CPCCR_CFG & ~(0xff << 24))
		| (7 << 20);
	cpm_outl(cpccr,CPM_CPCCR);
	while(cpm_inl(CPM_CPCSR) & 0x7);

	/* change sel */
	cpccr = (CPCCR_CFG & (0xff << 24)) | (cpm_inl(CPM_CPCCR) & ~(0xff << 24));
	cpm_outl(cpccr,CPM_CPCCR);
	debug("cppcr 0x%x\n",cpm_inl(CPM_CPCCR));
}

/* pllfreq align*/
static int inline align_pll(unsigned pllfreq, unsigned alfreq)
{
	int div = 0;
	if (!(pllfreq%alfreq)) {
		div = pllfreq/alfreq ? pllfreq/alfreq : 1;
	} else {
		error("pll freq is not integer times than cpu freq or/and ddr freq");
		asm volatile ("wait\n\t");
	}
	return alfreq * div;
}

/* Least Common Multiple */
static unsigned int lcm(unsigned int a, unsigned int b, unsigned int limit)
{
	unsigned int lcm_unit = a > b ? a : b;
	unsigned int lcm_resv = a > b ? b : a;
	unsigned int lcm = lcm_unit;;

	debug("caculate lcm :a(cpu:%d) and b(ddr%d) 's\t", a, b);
	while (lcm%lcm_resv &&  lcm < limit)
		lcm += lcm_unit;

	if (lcm%lcm_resv) {
		error("\n a(cpu %d), b(ddr %d) :	\
				Can not find Least Common Multiple in range of limit\n",
				a, b);
		asm volatile ("wait\n\t");
	}
	debug("lcm is %d\n",lcm);
	return lcm;
}

static void final_fill_div(int cpll, int ddrpll)
{
	unsigned cpu_pll_freq = (cpll == APLL)? pll_cfg.apll_freq : pll_cfg.mpll_freq;
	unsigned Periph_pll_freq = (ddrpll == APLL) ? pll_cfg.apll_freq : pll_cfg.mpll_freq;
	unsigned l2cache_clk = 0;

	/*DDRDIV*/
	gd->arch.gi->ddr_div = Periph_pll_freq/gd->arch.gi->ddrfreq;
	/*cdiv*/
	pll_cfg.cdiv = cpu_pll_freq/gd->arch.gi->cpufreq;

	/*AHB0 AHB2 <= 250M*/
	/*PCLK 75M ~ 150M*/
	switch (Periph_pll_freq/100000000) {
	case 10 ... 12:
		pll_cfg.pdiv = 8;
		pll_cfg.h0div = 4;
		pll_cfg.h2div = 4;
		break;
	case 7 ... 9:
		pll_cfg.pdiv = 8;
		pll_cfg.h0div = 4;
		pll_cfg.h2div = 4;
		break;
	default:
		error("Periph pll freq %d is out of range\n", Periph_pll_freq);
	case 6:
		pll_cfg.pdiv = 6;
		pll_cfg.h0div = 3;
		pll_cfg.h2div = 3;
		break;
	}

	/*L2CACHE <= 1.5 (AHB0 || AHB2)  && L2CACHE <= CPU*/
	l2cache_clk = (Periph_pll_freq/pll_cfg.h0div) * 3/2;
	l2cache_clk = l2cache_clk >= gd->arch.gi->cpufreq ? gd->arch.gi->cpufreq : l2cache_clk;

	printf("l2cache_clk = %d\n",l2cache_clk);

	pll_cfg.l2div = cpu_pll_freq/l2cache_clk;

	printf("pll_cfg.pdiv = %d, pll_cfg.h2div = %d, pll_cfg.h0div = %d, pll_cfg.cdiv = %d, pll_cfg.l2div = %d\n",
			pll_cfg.pdiv,pll_cfg.h2div,pll_cfg.h0div,pll_cfg.cdiv,pll_cfg.l2div);
	return;
}

static int freq_correcting(void)
{
	unsigned int pll_freq = 0;
	pll_cfg.mpll_freq = CONFIG_SYS_MPLL_FREQ > 0 ? CONFIG_SYS_MPLL_FREQ : 0;
	pll_cfg.apll_freq = CONFIG_SYS_APLL_FREQ > 0 ? CONFIG_SYS_APLL_FREQ : 0;

	if (!gd->arch.gi->cpufreq && !gd->arch.gi->ddrfreq) {
		error("cpufreq = %d and ddrfreq = %d can not be zero, check board config\n",
				gd->arch.gi->cpufreq,gd->arch.gi->ddrfreq);
		asm volatile ("wait\n\t");
	}

#define SEL_MAP(cpu,ddr) ((cpu<<16)|(ddr&0xffff))
#define PLL_MAXVAL 2400000000UL
	switch (SEL_MAP(CONFIG_CPU_SEL_PLL,CONFIG_DDR_SEL_PLL)) {
	case SEL_MAP(APLL,APLL):
		pll_freq = lcm(gd->arch.gi->cpufreq, gd->arch.gi->ddrfreq, PLL_MAXVAL);
		pll_cfg.apll_freq = align_pll(pll_cfg.apll_freq,pll_freq);
		final_fill_div(APLL, APLL);
		break;
	case SEL_MAP(MPLL,MPLL):
		pll_freq = lcm(gd->arch.gi->cpufreq, gd->arch.gi->ddrfreq, PLL_MAXVAL);
		pll_cfg.mpll_freq = align_pll(pll_cfg.mpll_freq, pll_freq);
		final_fill_div(MPLL, MPLL);
		break;
	case SEL_MAP(APLL,MPLL):
		pll_cfg.mpll_freq = align_pll(pll_cfg.mpll_freq, gd->arch.gi->ddrfreq);
		pll_cfg.apll_freq = align_pll(pll_cfg.apll_freq, gd->arch.gi->cpufreq);
		final_fill_div(APLL, MPLL);
		break;
	case SEL_MAP(MPLL,APLL):
		pll_cfg.apll_freq = align_pll(pll_cfg.apll_freq, gd->arch.gi->ddrfreq);
		pll_cfg.mpll_freq = align_pll(pll_cfg.mpll_freq, gd->arch.gi->cpufreq);
		final_fill_div(MPLL, APLL);
		break;
	}
#undef SEL_MAP
#undef PLL_MAXVAL
	return 0;
}

#if 0
void pll_test(int pll)
{
	unsigned i = 0, count = 0;
	while (1) {
		for (i = 24000000; i <= 1200000000; i += 24000000) {
			pll_set(pll,i);
			debug("time = %d ,apll = %d\n", count * 100 + i/100000000, clk_get_rate(pll));
		}
		for (i = 1200000000; i >= 24000000 ; i -= 24000000) {
			pll_set(pll,i);
			debug("time = %d, apll = %d\n", count * 100 + 50 + i/100000000, clk_get_rate(pll));
		}
		count++;
	}
}
#endif

int pll_init(void)
{
	freq_correcting();
	pll_set(APLL,pll_cfg.apll_freq);
	pll_set(MPLL,pll_cfg.mpll_freq);
	cpccr_init();
	print_clock();
	return 0;
}
