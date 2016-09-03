/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 ******************************************************************************/

#define _HAL_INTF_C_
#include <osdep_service.h>
#include <drv_types.h>
#include <hal_intf.h>
#include <usb_hal.h>

void rtw_hal_read_chip_version(struct adapter *adapt)
{
	if (adapt->HalFunc.read_chip_version)
		adapt->HalFunc.read_chip_version(adapt);
}

void rtw_hal_free_data(struct adapter *adapt)
{
	if (adapt->HalFunc.free_hal_data)
		adapt->HalFunc.free_hal_data(adapt);
}

void rtw_hal_dm_init(struct adapter *adapt)
{
	if (adapt->HalFunc.dm_init)
		adapt->HalFunc.dm_init(adapt);
}

u32 rtw_hal_power_on(struct adapter *adapt)
{
	if (adapt->HalFunc.hal_power_on)
		return adapt->HalFunc.hal_power_on(adapt);
	return _FAIL;
}

uint	 rtw_hal_init(struct adapter *adapt)
{
	uint	status = _SUCCESS;

	adapt->hw_init_completed = false;

	status = rtl8188eu_hal_init(adapt);

	if (status == _SUCCESS) {
		adapt->hw_init_completed = true;

		if (adapt->registrypriv.notch_filter == 1)
			rtw_hal_notch_filter(adapt, 1);

		rtw_hal_reset_security_engine(adapt);
	} else {
		adapt->hw_init_completed = false;
		DBG_88E("rtw_hal_init: hal__init fail\n");
	}

	RT_TRACE(_module_hal_init_c_, _drv_err_,
		 ("-rtl871x_hal_init:status=0x%x\n", status));

	return status;
}

uint rtw_hal_deinit(struct adapter *adapt)
{
	uint	status = _SUCCESS;

	status = rtl8188eu_hal_deinit(adapt);

	if (status == _SUCCESS)
		adapt->hw_init_completed = false;
	else
		DBG_88E("\n rtw_hal_deinit: hal_init fail\n");

	return status;
}

void rtw_hal_set_odm_var(struct adapter *adapt,
			 enum hal_odm_variable var, void *val1,
			 bool set)
{
	if (adapt->HalFunc.SetHalODMVarHandler)
		adapt->HalFunc.SetHalODMVarHandler(adapt, var,
						      val1, set);
}

void rtw_hal_update_ra_mask(struct adapter *adapt, u32 mac_id, u8 rssi_level)
{
	struct mlme_priv *pmlmepriv = &(adapt->mlmepriv);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == true) {
#ifdef CONFIG_88EU_AP_MODE
		struct sta_info *psta = NULL;
		struct sta_priv *pstapriv = &adapt->stapriv;

		if ((mac_id-1) > 0)
			psta = pstapriv->sta_aid[(mac_id-1) - 1];
		if (psta)
			add_RATid(adapt, psta, 0);/* todo: based on rssi_level*/
#endif
	} else {
		UpdateHalRAMask8188EUsb(adapt, mac_id, rssi_level);
	}
}

void rtw_hal_add_ra_tid(struct adapter *adapt, u32 bitmap, u8 arg,
			u8 rssi_level)
{
	if (adapt->HalFunc.Add_RateATid)
		adapt->HalFunc.Add_RateATid(adapt, bitmap, arg,
					       rssi_level);
}

u32 rtw_hal_read_rfreg(struct adapter *adapt, enum rf_radio_path rfpath,
		       u32 regaddr, u32 bitmask)
{
	u32 data = 0;

	if (adapt->HalFunc.read_rfreg)
		data = adapt->HalFunc.read_rfreg(adapt, rfpath, regaddr,
						    bitmask);
	return data;
}

void rtw_hal_set_bwmode(struct adapter *adapt,
			enum ht_channel_width bandwidth, u8 offset)
{
	if (adapt->HalFunc.set_bwmode_handler)
		adapt->HalFunc.set_bwmode_handler(adapt, bandwidth,
						     offset);
}

void rtw_hal_set_chan(struct adapter *adapt, u8 channel)
{
	if (adapt->HalFunc.set_channel_handler)
		adapt->HalFunc.set_channel_handler(adapt, channel);
}

void rtw_hal_dm_watchdog(struct adapter *adapt)
{
	if (adapt->HalFunc.hal_dm_watchdog)
		adapt->HalFunc.hal_dm_watchdog(adapt);
}

u8 rtw_hal_antdiv_before_linked(struct adapter *adapt)
{
	if (adapt->HalFunc.AntDivBeforeLinkHandler)
		return adapt->HalFunc.AntDivBeforeLinkHandler(adapt);
	return false;
}

void rtw_hal_antdiv_rssi_compared(struct adapter *adapt,
				  struct wlan_bssid_ex *dst,
				  struct wlan_bssid_ex *src)
{
	if (adapt->HalFunc.AntDivCompareHandler)
		adapt->HalFunc.AntDivCompareHandler(adapt, dst, src);
}

void rtw_hal_sreset_init(struct adapter *adapt)
{
	if (adapt->HalFunc.sreset_init_value)
		adapt->HalFunc.sreset_init_value(adapt);
}

void rtw_hal_notch_filter(struct adapter *adapter, bool enable)
{
	if (adapter->HalFunc.hal_notch_filter)
		adapter->HalFunc.hal_notch_filter(adapter, enable);
}

void rtw_hal_reset_security_engine(struct adapter *adapter)
{
	if (adapter->HalFunc.hal_reset_security_engine)
		adapter->HalFunc.hal_reset_security_engine(adapter);
}
