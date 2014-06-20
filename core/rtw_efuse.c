/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#define _RTW_EFUSE_C_

#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_efuse.h>
#include <usb_ops_linux.h>

/*------------------------Define local variable------------------------------*/
u8 fakeEfuseBank;
u32 fakeEfuseUsedBytes;
u8 fakeEfuseContent[EFUSE_MAX_HW_SIZE] = {0};
u8 fakeEfuseInitMap[EFUSE_MAX_MAP_LEN] = {0};
u8 fakeEfuseModifiedMap[EFUSE_MAX_MAP_LEN] = {0};

u32 BTEfuseUsedBytes;
u8 BTEfuseContent[EFUSE_MAX_BT_BANK][EFUSE_MAX_HW_SIZE];
u8 BTEfuseInitMap[EFUSE_BT_MAX_MAP_LEN] = {0};
u8 BTEfuseModifiedMap[EFUSE_BT_MAX_MAP_LEN] = {0};

u32 fakeBTEfuseUsedBytes;
u8 fakeBTEfuseContent[EFUSE_MAX_BT_BANK][EFUSE_MAX_HW_SIZE];
u8 fakeBTEfuseInitMap[EFUSE_BT_MAX_MAP_LEN] = {0};
u8 fakeBTEfuseModifiedMap[EFUSE_BT_MAX_MAP_LEN] = {0};
/*------------------------Define local variable------------------------------*/

/*  */
#define REG_EFUSE_CTRL		0x0030
#define EFUSE_CTRL			REG_EFUSE_CTRL		/*  E-Fuse Control. */
/*  */

/*  11/16/2008 MH Add description. Get current efuse area enabled word!!. */
u8
Efuse_CalculateWordCnts(u8 word_en)
{
	u8 word_cnts = 0;
	if (!(word_en & BIT(0)))
		word_cnts++; /*  0 : write enable */
	if (!(word_en & BIT(1)))
		word_cnts++;
	if (!(word_en & BIT(2)))
		word_cnts++;
	if (!(word_en & BIT(3)))
		word_cnts++;
	return word_cnts;
}

u8 efuse_OneByteRead(struct adapter *pAdapter, u16 addr, u8 *data, bool pseudo)
{
	u8 tmpidx = 0;
	u8 result;

	/*  -----------------e-fuse reg ctrl --------------------------------- */
	/* address */
	usb_write8(pAdapter, EFUSE_CTRL+1, (u8)(addr & 0xff));
	usb_write8(pAdapter, EFUSE_CTRL+2, ((u8)((addr>>8) & 0x03)) |
		   (usb_read8(pAdapter, EFUSE_CTRL+2) & 0xFC));

	usb_write8(pAdapter, EFUSE_CTRL+3,  0x72);/* read cmd */

	while (!(0x80 & usb_read8(pAdapter, EFUSE_CTRL+3)) && (tmpidx < 100))
		tmpidx++;
	if (tmpidx < 100) {
		*data = usb_read8(pAdapter, EFUSE_CTRL);
		result = true;
	} else {
		*data = 0xff;
		result = false;
	}
	return result;
}

/*  11/16/2008 MH Write one byte to reald Efuse. */
u8 efuse_OneByteWrite(struct adapter *pAdapter, u16 addr, u8 data, bool pseudo)
{
	u8 tmpidx = 0;
	u8 result;

	/*  -----------------e-fuse reg ctrl --------------------------------- */
	/* address */
	usb_write8(pAdapter, EFUSE_CTRL+1, (u8)(addr&0xff));
	usb_write8(pAdapter, EFUSE_CTRL+2,
		   (usb_read8(pAdapter, EFUSE_CTRL+2) & 0xFC) |
		   (u8)((addr>>8) & 0x03));
	usb_write8(pAdapter, EFUSE_CTRL, data);/* data */

	usb_write8(pAdapter, EFUSE_CTRL+3, 0xF2);/* write cmd */

	while ((0x80 &  usb_read8(pAdapter, EFUSE_CTRL+3)) && (tmpidx < 100))
		tmpidx++;

	if (tmpidx < 100)
		result = true;
	else
		result = false;

	return result;
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataRead
 *
 * Overview:	Read allowed word in current efuse section data.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008	MHC		Create Version 0.
 * 11/21/2008	MHC		Fix Write bug when we only enable late word.
 *
 *---------------------------------------------------------------------------*/
void efuse_WordEnableDataRead(u8 word_en, u8 *sourdata, u8 *targetdata)
{
	if (!(word_en&BIT(0))) {
		targetdata[0] = sourdata[0];
		targetdata[1] = sourdata[1];
	}
	if (!(word_en&BIT(1))) {
		targetdata[2] = sourdata[2];
		targetdata[3] = sourdata[3];
	}
	if (!(word_en&BIT(2))) {
		targetdata[4] = sourdata[4];
		targetdata[5] = sourdata[5];
	}
	if (!(word_en&BIT(3))) {
		targetdata[6] = sourdata[6];
		targetdata[7] = sourdata[7];
	}
}

static u8 efuse_read8(struct adapter *padapter, u16 address, u8 *value)
{
	return efuse_OneByteRead(padapter, address, value, false);
}

static u8 efuse_write8(struct adapter *padapter, u16 address, u8 *value)
{
	return efuse_OneByteWrite(padapter, address, *value, false);
}

/*
 * read/wirte raw efuse data
 */
u8 rtw_efuse_access(struct adapter *padapter, u8 write, u16 start_addr, u16 cnts, u8 *data)
{
	int i = 0;
	u16 real_content_len = 0, max_available_size = 0;
	u8 res = _FAIL;
	u8 (*rw8)(struct adapter *, u16, u8*);

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_REAL_CONTENT_LEN, (void *)&real_content_len);
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size);

	if (start_addr > real_content_len)
		return _FAIL;

	if (write) {
		if ((start_addr + cnts) > max_available_size)
			return _FAIL;
		rw8 = &efuse_write8;
	} else {
		rw8 = &efuse_read8;
	}

	Efuse_PowerSwitch(padapter, write, true);

	/*  e-fuse one byte read / write */
	for (i = 0; i < cnts; i++) {
		if (start_addr >= real_content_len) {
			res = _FAIL;
			break;
		}

		res = rw8(padapter, start_addr++, data++);
		if (_FAIL == res)
			break;
	}

	Efuse_PowerSwitch(padapter, write, false);

	return res;
}
/*  */
u16 efuse_GetMaxSize(struct adapter *padapter)
{
	u16 max_size;
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI , TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_size);
	return max_size;
}
/*  */
u8 efuse_GetCurrentSize(struct adapter *padapter, u16 *size)
{
	Efuse_PowerSwitch(padapter, false, true);
	*size = Efuse_GetCurrentSize(padapter, false);
	Efuse_PowerSwitch(padapter, false, false);

	return _SUCCESS;
}
/*  */
u8 rtw_efuse_map_read(struct adapter *padapter, u16 addr, u16 cnts, u8 *data)
{
	u16 mapLen = 0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	Efuse_PowerSwitch(padapter, false, true);

	efuse_ReadEFuse(padapter, EFUSE_WIFI, addr, cnts, data, false);

	Efuse_PowerSwitch(padapter, false, false);

	return _SUCCESS;
}

/*  */
u8 rtw_efuse_map_write(struct adapter *padapter, u16 addr, u16 cnts, u8 *data)
{
	u8 offset, word_en;
	u8 *map;
	u8 newdata[PGPKT_DATA_SIZE + 1];
	s32	i, idx;
	u8 ret = _SUCCESS;
	u16 mapLen = 0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	map = rtw_zmalloc(mapLen);
	if (map == NULL)
		return _FAIL;

	ret = rtw_efuse_map_read(padapter, 0, mapLen, map);
	if (ret == _FAIL)
		goto exit;

	Efuse_PowerSwitch(padapter, true, true);

	offset = (addr >> 3);
	word_en = 0xF;
	_rtw_memset(newdata, 0xFF, PGPKT_DATA_SIZE + 1);
	i = addr & 0x7;	/*  index of one package */
	idx = 0;	/*  data index */

	if (i & 0x1) {
		/*  odd start */
		if (data[idx] != map[addr+idx]) {
			word_en &= ~BIT(i >> 1);
			newdata[i-1] = map[addr+idx-1];
			newdata[i] = data[idx];
		}
		i++;
		idx++;
	}
	do {
		for (; i < PGPKT_DATA_SIZE; i += 2) {
			if (cnts == idx)
				break;
			if ((cnts - idx) == 1) {
				if (data[idx] != map[addr+idx]) {
					word_en &= ~BIT(i >> 1);
					newdata[i] = data[idx];
					newdata[i+1] = map[addr+idx+1];
				}
				idx++;
				break;
			} else {
				if ((data[idx] != map[addr+idx]) ||
				    (data[idx+1] != map[addr+idx+1])) {
					word_en &= ~BIT(i >> 1);
					newdata[i] = data[idx];
					newdata[i+1] = data[idx + 1];
				}
				idx += 2;
			}
			if (idx == cnts)
				break;
		}

		if (word_en != 0xF) {
			ret = Efuse_PgPacketWrite(padapter, offset, word_en, newdata, false);
			DBG_88E("offset=%x\n", offset);
			DBG_88E("word_en=%x\n", word_en);

			for (i = 0; i < PGPKT_DATA_SIZE; i++)
				DBG_88E("data=%x \t", newdata[i]);
			if (ret == _FAIL)
				break;
		}

		if (idx == cnts)
			break;

		offset++;
		i = 0;
		word_en = 0xF;
		_rtw_memset(newdata, 0xFF, PGPKT_DATA_SIZE);
	} while (1);

	Efuse_PowerSwitch(padapter, true, false);
exit:
	kfree(map);
	return ret;
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_ShadowRead1Byte
 *			efuse_ShadowRead2Byte
 *			efuse_ShadowRead4Byte
 *
 * Overview:	Read from efuse init map by one/two/four bytes !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void
efuse_ShadowRead1Byte(
		struct adapter *pAdapter,
		u16 Offset,
		u8 *Value)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(pAdapter);

	*Value = pEEPROM->efuse_eeprom_data[Offset];

}	/*  EFUSE_ShadowRead1Byte */

/* Read Two Bytes */
static void
efuse_ShadowRead2Byte(
		struct adapter *pAdapter,
		u16 Offset,
		u16 *Value)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(pAdapter);

	*Value = pEEPROM->efuse_eeprom_data[Offset];
	*Value |= pEEPROM->efuse_eeprom_data[Offset+1]<<8;

}	/*  EFUSE_ShadowRead2Byte */

/* Read Four Bytes */
static void
efuse_ShadowRead4Byte(
		struct adapter *pAdapter,
		u16 Offset,
		u32 *Value)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(pAdapter);

	*Value = pEEPROM->efuse_eeprom_data[Offset];
	*Value |= pEEPROM->efuse_eeprom_data[Offset+1]<<8;
	*Value |= pEEPROM->efuse_eeprom_data[Offset+2]<<16;
	*Value |= pEEPROM->efuse_eeprom_data[Offset+3]<<24;

}	/*  efuse_ShadowRead4Byte */

/*-----------------------------------------------------------------------------
 * Function:	Efuse_ReadAllMap
 *
 * Overview:	Read All Efuse content
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/11/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void Efuse_ReadAllMap(struct adapter *pAdapter, u8 efuseType, u8 *Efuse, bool pseudo)
{
	u16 mapLen = 0;

	Efuse_PowerSwitch(pAdapter, false, true);

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	efuse_ReadEFuse(pAdapter, efuseType, 0, mapLen, Efuse, pseudo);

	Efuse_PowerSwitch(pAdapter, false, false);
}

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowMapUpdate
 *
 * Overview:	Transfer current EFUSE content to shadow init and modify map.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/13/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void EFUSE_ShadowMapUpdate(
	struct adapter *pAdapter,
	u8 efuseType,
	bool pseudo)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(pAdapter);
	u16 mapLen = 0;

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if (pEEPROM->bautoload_fail_flag)
		_rtw_memset(pEEPROM->efuse_eeprom_data, 0xFF, mapLen);
	else
		Efuse_ReadAllMap(pAdapter, efuseType, pEEPROM->efuse_eeprom_data, pseudo);
} /*  EFUSE_ShadowMapUpdate */

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowRead
 *
 * Overview:	Read from efuse init map !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void EFUSE_ShadowRead(struct adapter *pAdapter, u8 Type, u16 Offset, u32 *Value)
{
	if (Type == 1)
		efuse_ShadowRead1Byte(pAdapter, Offset, (u8 *)Value);
	else if (Type == 2)
		efuse_ShadowRead2Byte(pAdapter, Offset, (u16 *)Value);
	else if (Type == 4)
		efuse_ShadowRead4Byte(pAdapter, Offset, (u32 *)Value);

}	/*  EFUSE_ShadowRead */
