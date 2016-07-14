/*
 *  Copyright (C) 2013 NEXELL SOC Lab.
 *  BongKwan Kook <kook@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <errno.h>
#include <power/pmic.h>
#include <power/battery.h>
#include "nxe2000-private.h"

#include <nxe2000_power.h>

static struct battery battery_nxe2000;

static int power_battery_charge(struct pmic *bat)
{
	struct power_battery *p_bat = bat->pbat;
	struct battery *battery = p_bat->bat;
	int k;

	if (bat->chrg->chrg_state(p_bat->chrg, CHARGER_ENABLE, 450))
		return -1;

	for (k = 0; bat->chrg->chrg_bat_present(p_bat->chrg) &&
		     bat->chrg->chrg_type(p_bat->muic, 1) &&
		     battery->state_of_chrg < 100; k++) {
		udelay(10000000);
		puts(".");
		bat->fg->fg_battery_update(p_bat->fg, bat);

		if (k == 100) {
			debug(" %d [V]", battery->voltage_uV);
			puts("\n");
			k = 0;
		}
	}

	bat->chrg->chrg_state(p_bat->chrg, CHARGER_DISABLE, 0);

	return 0;
}

static int power_battery_init_nxe2000(struct pmic *bat_,
				    struct pmic *fg_,
				    struct pmic *chrg_,
				    struct pmic *muic_)
{
	bat_->pbat->fg = fg_;
	bat_->pbat->chrg = chrg_;
	bat_->pbat->muic = muic_;

	bat_->fg = fg_->fg;
	bat_->chrg = chrg_->chrg;
	if (muic_)
		bat_->chrg->chrg_type = muic_->chrg->chrg_type;
	return 0;
}

static struct power_battery power_bat_nxe2000 = {
	.bat = &battery_nxe2000,
	.battery_init = power_battery_init_nxe2000,
	.battery_charge = power_battery_charge,
};

int power_bat_init(unsigned char bus)
{
	static const char name[] = "BAT_NXE2000";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	debug("Board BAT init\n");

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = NXE2000_NUM_OF_REGS;
	p->hw.i2c.addr = NXE2000_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	p->pbat = &power_bat_nxe2000;
	return 0;
}
