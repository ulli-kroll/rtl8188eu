bool rtl88eu_phy_mac_config(struct adapter *adapt);
bool rtl88eu_phy_rf_config(struct adapter *adapt);
bool rtl88eu_phy_bb_config(struct adapter *adapt);

u32 phy_query_bb_reg(struct adapter *adapt, u32 regaddr, u32 bitmask);
void phy_set_bb_reg(struct adapter *adapt, u32 regaddr, u32 bitmask, u32 data);
u32 phy_query_rf_reg(struct adapter *adapt, enum rf_radio_path rf_path,
		     u32 reg_addr, u32 bit_mask);
void phy_set_rf_reg(struct adapter *adapt, enum rf_radio_path rf_path,
		    u32 reg_addr, u32 bit_mask, u32 data);

void phy_set_tx_power_level(struct adapter *adapt, u8 channel);

void phy_set_bw_mode(struct adapter *adapt, enum ht_channel_width bandwidth,
		     unsigned char offset);
void phy_sw_chnl(struct adapter *adapt, u8 channel);
