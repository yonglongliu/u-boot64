
#include <common.h>
#include "dl_cmd_proc.h"

#ifdef CONFIG_EMMC_BOOT
#include "dl_emmc_operate.h"
#endif
#ifdef CONFIG_NAND_BOOT
#include "dl_nand_operate.h"
#endif

#include <boot_mode.h>
#include "asm/arch/check_reboot.h"
#include "otp_helper.h"
#include "asm/arch/common.h"
#include <malloc.h>


extern void fixup_pmic_items(void);

#include "root_inspect.h"

static root_stat_t root_stat;


static void _decode_packet_data(dl_packet_t *packet, uchar * partition_name, uint32_t * size, uint32_t * checksum)
{
	uint32_t  i = 0;
	uint16_t *data = (uint16_t *) (packet->body.content);
	*size = *(uint32_t *) (data + MAX_PARTITION_NAME_SIZE);
	if (NULL != checksum)
		*checksum = *(uint32_t *) (data + MAX_PARTITION_NAME_SIZE + 2);

	while (('\0' != *(data+i)) && (i < (MAX_PARTITION_NAME_SIZE-1))){
		partition_name[i] = *(data+i) & 0xFF;
		i++;
	}
	partition_name[i] = '\0';

	return;
}

static __inline dl_cmd_type_t convert_operate_status(int err)
{
	switch (err)
	{
		case OPERATE_SUCCESS:
			return BSL_REP_ACK;
		case OPERATE_INVALID_ADDR:
			return BSL_REP_DOWN_DEST_ERROR;
		case OPERATE_INVALID_SIZE:
			return BSL_REP_DOWN_SIZE_ERROR;
		case OPERATE_DEVICE_INIT_ERROR:
			return BSL_UNKNOWN_DEVICE;
		case OPERATE_INVALID_DEVICE_SIZE:
			return BSL_INVALID_DEVICE_SIZE;
		case OPERATE_INCOMPATIBLE_PART:
			return BSL_INCOMPATIBLE_PARTITION;
		case OPERATE_WRITE_ERROR:
			return BSL_WRITE_ERROR;
		case OPERATE_CHECKSUM_DIFF:
			return BSL_CHECKSUM_DIFF;

		default:
		    return BSL_REP_OPERATION_FAILED;
	}
}


static __inline void _send_reply(uint32_t err)
{
	dl_send_ack (convert_operate_status(err));
	return;
}

int _parse_repartition_header(uchar * data, REPARTITION_TABLE_INFO * info, uchar ** list)
{
	uchar *  pointer = data;
	/*magic number must be "par:", otherwise it must be the old version packet(version 0)*/
	if (*(uint32_t*)data != REPARTITION_HEADER_MAGIC) {
		info->version = 0;
		/*default unit in version 0 is MB*/
		info->unit = 0;
		*list = data;
		return 0;
	}

	/*   header format:
	  *	|  magic(4Byte) | Version(1Byte) | Unit(1Byte) | table count(1Byte)|Reserved(1Byte) |
	  *	table tag(4) | table offset(2)| table size(2)|
	  */
	pointer += 4;
	info->version = *pointer;
	pointer += 1;
	info->unit = *pointer;
	pointer += 1;
	info->table_count = *pointer;
	pointer += 1;
	info->reserved = *pointer;
	pointer += 1;
	info->table_tag = *(unsigned int *)pointer;
	pointer += 4;
	info->table_offset = *(unsigned short *)pointer;
	pointer += 2;
	info->table_size = *(unsigned short *)pointer;
	pointer += 2;
	printf("%s: version(%d),unit(%d), table_count(%d), table_tag(%d), table_offset(%d), table_size(%d)\n",
		__FUNCTION__, info->version, info->unit, info->table_count, info->table_tag, info->table_offset,
		info->table_size);
	*list = pointer;
	return 0;
}

int dl_cmd_write_start (dl_packet_t *packet, void *arg)
{
	uchar partition_name[MAX_PARTITION_NAME_SIZE] = {0};
	uint32_t size;
	uint32_t nv_checksum;
	int32_t           op_res = 1;

	_decode_packet_data(packet, partition_name, &size, &nv_checksum);

	debugf("partition_name=%s,size=0x%x,nvchecksum=0x%x\n", partition_name, size, nv_checksum);
	if((root_stat.root_flag == 1) && !strcmp(partition_name, "system")) {
		debugf("start clear root flag.\n");
		erase_rootflag(&root_stat);
	}
	op_res = dl_download_start(partition_name, size, nv_checksum);

	debugf("operate result =%d,send ack\n", op_res);
	_send_reply(op_res);
	return 0;
}

int dl_cmd_write_midst(dl_packet_t *packet, void *arg)
{
	int32_t           op_res = 1;

	//debugf("size=0x%x\n", packet->body.size);

	op_res = dl_download_midst(packet->body.size, (char *)(packet->body.content));
	//debugf("befsendack,time=%u ", SCI_GetTickCount());
	_send_reply(op_res);
	//debugf("operate result =%d,send ack\n", op_res);
	return 0;
}

int dl_cmd_write_end (dl_packet_t *packet, void *arg)
{
	int32_t           op_res = 1;
	op_res = dl_download_end();
	_send_reply(op_res);
	return 0;
}

int dl_cmd_read_start(dl_packet_t *packet, void *arg)
{
	uchar partition_name[MAX_PARTITION_NAME_SIZE] = {0};
	uint32_t size;
	int32_t           op_res = 0;

	_decode_packet_data(packet, partition_name, &size, NULL);

	debugf("partition_name=%s,size=0x%x\n", partition_name, size);
	op_res = dl_read_start(partition_name, size);

	debugf("operate result =%d,send ack\n", op_res);
	_send_reply(op_res);
	return 0;
}

int dl_cmd_read_midst(dl_packet_t *packet, void *arg)
{
	uint32_t *data = (uint32_t *) (packet->body.content);
	uint32_t size = *data;
	uint32_t off = *(data + 1);
	int32_t           op_res = 0;

	if (size > MAX_PKT_SIZE) {
		debug("Size:0x%x beyond MAX_PKT_SIZE(0x%x)\n",size,MAX_PKT_SIZE);
		dl_send_ack (BSL_REP_DOWN_SIZE_ERROR);
		return 0;
	}


	op_res = dl_read_midst(size, off, (uchar *)(packet->body.content));
	if (OPERATE_SUCCESS == op_res) {
		packet->body.type = BSL_REP_READ_FLASH;
		packet->body.size = size;
		//debugf("operate result =%d,send pkt size=%d\n", op_res, size);
		dl_send_packet (packet);
	} else {
		debugf("operate result =%d,send ack\n", op_res);
		_send_reply(op_res);
	}
	return 0;
}

int dl_cmd_read_end(dl_packet_t *packet, void *arg)
{
	int32_t op_res =0;
	
	op_res = dl_read_end();
	_send_reply(op_res);
	return 0;
}

int dl_cmd_erase(dl_packet_t *packet, void *arg)
{
	uchar partition_name[MAX_PARTITION_NAME_SIZE] = {0};
	uint32_t size;
	int32_t op_res =0;

	_decode_packet_data(packet, partition_name, &size, NULL);

	debugf("partition_name=%s,size=%u\n", partition_name, size);
	op_res = dl_erase(partition_name, size);

	debugf("operate result =%d,send ack\n", op_res);
	_send_reply(op_res);
	return 0;
}


int dl_cmd_repartition(dl_packet_t *pakcet, void *arg)
{
	uint16_t total_partition_num = 0;
	int32_t op_res =1;
	uint16_t size = pakcet->body.size;
	uchar * raw_data = pakcet->body.content;
	REPARTITION_TABLE_INFO rp_info;
	uchar * p_part_list = NULL;
	uint32_t part_cell_length = 0;

	_parse_repartition_header(raw_data, &rp_info, &p_part_list);

	if (0 == rp_info.version) {
		part_cell_length = REPARTITION_UNIT_LENGTH;
	} else {
		part_cell_length = REPARTITION_UNIT_LENGTH_V1;
		size = rp_info.table_size;
	}

	if (0 != (size % part_cell_length)) {
		printf("%s:recvd packet size(%d) error \n", __FUNCTION__,size);
		dl_send_packet (BSL_INCOMPATIBLE_PARTITION);
		return 0;
	}
	total_partition_num = size / part_cell_length;
	debugf("Partition total num:%d \n", total_partition_num);
	op_res = dl_repartition(p_part_list, total_partition_num, rp_info.version, rp_info.unit);
	_send_reply(op_res);
	return 0;
}


int dl_cmd_reboot (dl_packet_t *pakcet, void *arg)
{
	int32_t op_res = OPERATE_SUCCESS;
	debugf("before reboot ,send ack(%d)\n", op_res);
	_send_reply(op_res);
	reset_to_normal(CMD_NORMAL_MODE);
	return 0;
}

int dl_powerdown_device(dl_packet_t *packet, void *arg)
{

	int32_t op_res = OPERATE_SUCCESS;

	_send_reply(op_res);

	/* Wait until all characters are sent out. */
	udelay(200);

	/* Wait until the charger unconnected. */
	while(charger_connected());
	debugf("download end power down device\n");
	power_down_devices(0);
	while(1);
	return 0;
}



int dl_cmd_read_mcptype(dl_packet_t *packet, void *arg)
{

	uint16_t size ;


#ifdef CONFIG_NAND_BOOT
	dl_get_mcptype(packet->body.content, &size);
	packet->body.type = BSL_REP_READ_MCP_TYPE;
	packet->body.size = size;
#else
	packet->body.type = BSL_UNKNOWN_DEVICE;
	packet->body.size = 0;

#endif
	dl_send_packet (packet);
	return 1;


}

int dl_cmd_check_rootflag(dl_packet_t *packet, void *arg)
{
	u32 result;
	int32_t op_res = OPERATE_SUCCESS;

	result = get_rootflag(&root_stat);
	if(result == 0) {
		debugf("this phone have not been rooted from now.\n");
		_send_reply(op_res);
	} else {
		debugf("phone was brushed,rootflag:%d\n", result);
		dl_send_ack(BSL_PHONE_ROOTFLAG);
	}

	return 1;
}

int dl_cmd_get_uid(dl_packet_t *packet, void *arg)
{
	uint32_t efuse_blk0 = 0;
	uint32_t efuse_blk1 = 0;
	uint32_t * uid = (unsigned int *) (packet->body.content);

	efuse_blk0 = __ddie_efuse_read(0);
	efuse_blk0 &= ~(BIT_31);
	efuse_blk1 = __ddie_efuse_read(1);
	efuse_blk1 &= ~(BIT_31);
	debugf("Read efuse_blk0 = 0x%x, efuse_blk1 = 0x%x\n", efuse_blk0, efuse_blk1);

	packet->body.type = BSL_REP_READ_CHIP_UID;
	packet->body.size = 0x8;
	*uid = efuse_blk0;
	uid ++;
	*uid = efuse_blk1;
	dl_send_packet (packet);
	return 0;
}

int dl_cmd_read_ref_info(dl_packet_t *packet, void *arg)
{
	debugf("begin\n");
	char *buf = NULL;
	unsigned short size = packet->body.size;
	packet->body.type = BSL_REP_READ_REFINFO;
	if(size != REF_INFO_SIZE){
		errorf("ref info size does not match \n");
	} else {
		buf = malloc((size_t)size);
		if (buf == NULL) {
			errorf("malloc error, size = 0x%x \n", size);
		} else {
			memcpy(buf, packet->body.content, (size_t)size);
			dl_read_ref_info("miscdata", size, (uint64_t)REF_INFO_OFF,
				buf, (uchar *)(packet->body.content));
		}
		if(buf != NULL)
			free(buf);
	}
		dl_send_packet (packet);
	return 0;
}

