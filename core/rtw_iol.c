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

#include<rtw_iol.h>

int rtw_IOL_append_cmds(struct xmit_frame *xmit_frame, u8 *IOL_cmds, u32 cmd_len)
{
	struct pkt_attrib	*pattrib = &xmit_frame->attrib;
	u16 buf_offset;
	u32 ori_len;

	buf_offset = TXDESC_OFFSET;
	ori_len = buf_offset+pattrib->pktlen;

	/* check if the io_buf can accommodate new cmds */
	if (ori_len + cmd_len + 8 > MAX_XMITBUF_SZ) {
		DBG_88E("%s %u is large than MAX_XMITBUF_SZ:%u, can't accommodate new cmds\n",
			__func__ , ori_len + cmd_len + 8, MAX_XMITBUF_SZ);
		return _FAIL;
	}

	memcpy(xmit_frame->buf_addr + buf_offset + pattrib->pktlen, IOL_cmds, cmd_len);
	pattrib->pktlen += cmd_len;
	pattrib->last_txcmdsz += cmd_len;

	return _SUCCESS;
}

bool rtw_IOL_applied(struct adapter  *adapter)
{
	if (1 == adapter->registrypriv.fw_iol)
		return true;

	if ((2 == adapter->registrypriv.fw_iol) && (!adapter_to_dvobj(adapter)->ishighspeed))
		return true;
	return false;
}

int rtw_IOL_append_LLT_cmd(struct xmit_frame *xmit_frame, u8 page_boundary)
{
	return _SUCCESS;
}

int _rtw_IOL_append_WB_cmd(struct xmit_frame *xmit_frame, u16 addr, u8 value, u8 mask)
{
	struct ioreg_cfg cmd = {8, IOREG_CMD_WB_REG, 0x0, 0x0, 0x0};

	cmd.address = cpu_to_le16(addr);
	cmd.data = cpu_to_le32(value);

	if (mask != 0xFF) {
		cmd.length = 12;
		cmd.mask = cpu_to_le32(mask);
	}
	return rtw_IOL_append_cmds(xmit_frame, (u8 *)&cmd, cmd.length);
}

int _rtw_IOL_append_WRF_cmd(struct xmit_frame *xmit_frame, u8 rf_path, u16 addr, u32 value, u32 mask)
{
	struct ioreg_cfg cmd = {8, IOREG_CMD_W_RF, 0x0, 0x0, 0x0};

	cmd.address = cpu_to_le16((rf_path<<8) | ((addr) & 0xFF));
	cmd.data = cpu_to_le32(value);

	if (mask != 0x000FFFFF) {
		cmd.length = 12;
		cmd.mask =  cpu_to_le32(mask);
	}
	return rtw_IOL_append_cmds(xmit_frame, (u8 *)&cmd, cmd.length);
}

int rtw_IOL_append_END_cmd(struct xmit_frame *xmit_frame)
{
	struct ioreg_cfg cmd = {4, IOREG_CMD_END, cpu_to_le16(0xFFFF), cpu_to_le32(0xFF), 0x0};

	return rtw_IOL_append_cmds(xmit_frame, (u8 *)&cmd, 4);
}
