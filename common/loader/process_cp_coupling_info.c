#include <common.h>
#include <libfdt_env.h>
#include <libfdt.h>
#include "cp_mem_decoupling.h"
#include "loader_common.h"

extern int _boot_partition_read(block_dev_desc_t * dev, uchar * partition_name, uint32_t offsetsector, uint32_t size, uint8_t * buf);
static int do_parse_add_rsvmem_info(struct keyword *kwd, struct cp_mem_coupling_info *info, struct fdt_header *fdt);
static int do_parse_add_sipc_info(struct keyword *kwd, struct cp_mem_coupling_info *info, struct fdt_header *fdt);
static int do_parse_add_cproc_info(struct keyword *kwd, struct cp_mem_coupling_info *info, struct fdt_header *fdt);

static boot_image_required_t s_boot_image_cp_table[MAX_REGION_NUM] = {0};
static struct partition_name_info g_partition_info[MAX_REGION_NUM] = {0};

static struct cp_mem_coupling_info g_info;

#define CP_MEM_DECOUP_DEBUG
#define min(A,B) (((A) < (B)) ? (A) : (B))
#define TOSTRING(S...) #S
#define SPLICE(A,B) A,B
#define KEYWORD(kname, kvalue, func) \
	{{TOSTRING(kname), #kvalue}, func},

static struct keyword g_keyword[] = {
	KEYWORD(rsvmem, NULL, do_parse_add_rsvmem_info)
	KEYWORD(SPLICE(sprd,decoup), sipc-use-decoup, do_parse_add_sipc_info)
	KEYWORD(SPLICE(sprd,decoup), cproc-use-decoup, do_parse_add_cproc_info)
	NULL
};

static void print_fdt_rsvmem(struct fdt_header *fdt)
{
#ifdef CP_MEM_DECOUP_DEBUG
	char fdt_cmd[MAX_VALUE_LEN] = {0};

	sprintf(fdt_cmd, "fdt addr %p", fdt);
	run_command(fdt_cmd, 0);
	run_command("fdt rsvmem print", 0);
#endif
	return;
}

static void print_fdt_node(struct fdt_header *fdt, int node_offset)
{
#ifdef CP_MEM_DECOUP_DEBUG
	int index = 0;
	char fdt_cmd[MAX_VALUE_LEN] = {0};

	sprintf(fdt_cmd, "fdt addr %p", fdt);
	run_command(fdt_cmd, 0);

	memset(fdt_cmd, 0, sizeof(fdt_cmd));
	strcpy(fdt_cmd, "fdt print ");
	index = strlen(fdt_cmd);
	if(fdt_get_path(fdt, node_offset, fdt_cmd + index, sizeof(fdt_cmd) - index) == 0) {
		run_command(fdt_cmd, 0);
	}
#endif
	return;
}

static int property_init(char *name, uint32_t node_offset, struct property *prop)
{
	int num;

	if(NULL == name || NULL == prop) {
		printf("%s: param error\n", __func__);
		return -1;
	}

	memset(prop, 0, sizeof(struct property));
	num = min(sizeof(prop->name) - 1, strlen(name));
	strncpy(prop->name, name, num);
	prop->offset = node_offset;

	return 0;
}

static void count_addr_size_cells(struct fdt_header *fdt, int offset, int *addrc, int *sizec)
{
	const fdt32_t *prop;
	int parentoffset;

	parentoffset = fdt_parent_offset(fdt, offset);
	if(parentoffset < 0) {
		printf("%s: offset = 0x%x is a root node offset\n", __func__, offset);
		*addrc = 2;
		*sizec = 1;
		return ;
	}

	if (addrc) {
		prop = fdt_getprop(fdt, parentoffset, "#address-cells", NULL);
		if (prop)
			*addrc = be32_to_cpup(prop);
		else
			*addrc = 2;
	}

	if (sizec) {
		prop = fdt_getprop(fdt, parentoffset, "#size-cells", NULL);
		if (prop)
			*sizec = be32_to_cpup(prop);
		else
			*sizec = 1;
	}

	return ;
}

static int fill_prop_value(int type, void *value, int len, struct property *prop)
{
	int i, n, lenp;
	uint32_t val;
	char *s = NULL;
	char *p = NULL;

	if(NULL == value || NULL == prop || 0 == len) {
		printf("%s: param error\n", __func__);
		return -1;
	}

	if(len > sizeof(prop->value)) {
		printf("%s: <%s> fill value too big, len = %d\n", __func__, prop->name, len);
		return 0;
	}

	lenp = prop->len;
	p = prop->value + lenp;

	if(type == str_t) { //property value is string type
		strcpy(p, value);
		lenp += strlen(value) + 1;

	} else {
		n = len / sizeof(uint32_t);
		if(!n)
			n = 1;

		memset(p, 0, n * sizeof(uint32_t));

		p += n * sizeof(uint32_t);
		lenp += n * sizeof(uint32_t);

		for(s = (char *)value + len ; n > 0; n--) {
			s -= sizeof(uint32_t);
			p -= sizeof(uint32_t);
			memcpy(&val, s, sizeof(uint32_t));
			val = __cpu_to_be32(val);
			memcpy(p, &val, sizeof(uint32_t));
		}
	}

	prop->len = lenp;
	prop->flag = filled_prop;

	return 0;
}

static int fill_cell_prop_value(void *value, uint32_t size, int cell, struct property *prop)
{
	int i, n, delta;
	char temp[64];
	char *p = NULL;

	if(!value || !prop) {
		printf("%s: param error\n", __func__);
		return -1;
	}

	p = (char *)value;
	n = cell * sizeof(uint32_t) < size ? cell * sizeof(uint32_t) : size;
	delta = cell * sizeof(uint32_t) - sizeof(uint32_t);
	memset(temp, 0, sizeof(temp));
	for(i = cell; i > 0; i--) {
		memcpy(temp + delta, p, sizeof(uint32_t));
		delta -= sizeof(uint32_t);
		p += sizeof(uint32_t);
	}

	return fill_prop_value(num_t, temp, cell * sizeof(uint32_t), prop);
}

static int fill_reg_prop_value(uint64_t base, uint64_t size, int ac, int sc, struct property *prop)
{
	if(ac > 2 || sc > 2 || !prop) {
		printf("%s: param error\n", __func__);
		return -1;
	}

	if(size == 0) {
		printf("%s: size = 0, invalid\n", __func__);
		return 0;
	}

	if(fill_cell_prop_value(&base, sizeof(base), ac, prop)) {
		printf("%s: fill base to %s prop failed.\n", __func__, prop->name);
		return -1;
	}

	if(fill_cell_prop_value(&size, sizeof(size), sc, prop)) {
		printf("%s: fill size to %s prop failed.\n", __func__, prop->name);
		return -1;
	}

	return 0;
}

static int fill_ranges_prop_value(uint64_t offs, uint64_t base, uint64_t size, int c_ac, int ac, int c_sc, struct property *prop)
{
	if(ac > 2 || c_ac > 2 || c_sc > 2 || !prop) {
		printf("%s: param error\n", __func__);
		return -1;
	}

	if(fill_cell_prop_value(&offs, sizeof(offs), c_ac, prop)) {
		printf("%s: fill base to %s prop failed.\n", __func__, prop->name);
		return -1;
	}

	if(fill_cell_prop_value(&base, sizeof(base), ac, prop)) {
		printf("%s: fill base to %s prop failed.\n", __func__, prop->name);
		return -1;
	}

	if(fill_cell_prop_value(&size, sizeof(size), c_sc, prop)) {
		printf("%s: fill size to %s prop failed.\n", __func__, prop->name);
		return -1;
	}

	return 0;
}

static int do_add_prop_to_fdt(struct fdt_header *fdt, int mode, struct property *prop)
{
	int ret;

	if(NULL == prop || NULL == fdt) {
		printf("%s: param error\n", __func__);
		return -1;
	}

	if(filled_prop != prop->flag) {
		printf("%s: %s prop is bad or no value, do not add to fdt\n", __func__, prop->name);
		return -1;
	}

	if(mode != 0) {
		ret = fdt_appendprop(fdt, prop->offset, prop->name, prop->value, prop->len);
	} else {
		ret = fdt_setprop(fdt, prop->offset, prop->name, prop->value, prop->len);
	}

	if (ret < 0) {
		printf("%s: add prop %s to %s failed, reason: %s\n",
			__func__, prop->name, fdt_get_name(fdt, prop->offset, 0), fdt_strerror(ret));
		return -1;
	}

	return 0;
}

static int do_parse_add_sipc_info(struct keyword *kwd, struct cp_mem_coupling_info *info, struct fdt_header *fdt)
{
	int node_offset, c_node_offset;
	uint32_t offs;
	uint32_t addr_cells = 2, size_cells = 1;
	uint32_t c_addr_cells = 2, c_size_cells = 1;
	struct property reg, c_reg, ranges;
	struct comm_info_conf *comm_info = NULL;
	struct keyinfo *key = NULL;

	if(NULL == kwd || NULL == info || NULL == fdt) {
		printf("%s: param error\n", __func__);
		goto err;
	}

	comm_info = &info->comm_info;
	key = &kwd->key;
	node_offset = fdt_node_offset_by_prop_value(fdt, 0, key->name, key->value, strlen(key->value) + 1);
	if(node_offset < 0) {
		printf("%s: the sipc node do not support decoupling\n", __func__);
		//goto err;
		return 1;
	}

	c_node_offset = fdt_first_subnode(fdt, node_offset);
	if(c_node_offset < 0) {
		printf("%s: the sipc child node not found\n", __func__);
		goto err;
	}

	printf("start update sipc node info\n");
	print_fdt_node(fdt, node_offset);

	count_addr_size_cells(fdt, node_offset, &addr_cells, &size_cells);
	printf("%s: a_cells = %d, s_cells = %d\n", __func__, addr_cells, size_cells);

	count_addr_size_cells(fdt, c_node_offset, &c_addr_cells, &c_size_cells);
	printf("%s: ca_cells: %d, cs_cells: %d\n", __func__, c_addr_cells, c_size_cells);

	property_init("reg", c_node_offset, &c_reg);
	property_init("reg", node_offset, &reg);
	property_init("ranges", node_offset, &ranges);
	printf("%s: property init done\n");

	printf("%s: smem.base: 0x%p, smem.size: 0x%p\n", __func__,
		comm_info->smem.base, comm_info->smem.size);
	printf("%s: srbuf.base: 0x%p, srbuf.size: 0x%p\n", __func__,
		comm_info->srbuf.base, comm_info->srbuf.size);

	if(fill_reg_prop_value(comm_info->smem.base, comm_info->smem.size, addr_cells, size_cells, &reg) < 0) {
		printf("%s: fill reg prop value failed\n", __func__);
		goto err;
	}

	if(do_add_prop_to_fdt(fdt, 0, &reg) < 0) {
		printf("%s: update reg property to sipc node failed\n", __func__);
		goto err;
	}

	offs = comm_info->smem.base - PHY_BASE_ADDR;
	if(fill_reg_prop_value(offs, comm_info->smem.size, c_addr_cells, c_size_cells, &c_reg) < 0) {
		printf("%s: fill smem base and size to child node reg prop value failed\n", __func__);
		goto err;
	}

	if(fill_ranges_prop_value(offs, comm_info->smem.base, comm_info->smem.size, c_addr_cells, addr_cells, c_size_cells, &ranges) < 0) {
		printf("%s: fill smem base and size to range prop failed\n", __func__);
		goto err;
	}

	offs = comm_info->srbuf.base - PHY_BASE_ADDR;
	if(fill_reg_prop_value(offs, comm_info->srbuf.size, c_addr_cells, c_size_cells, &c_reg) < 0) {
		printf("%s: fill srbuf base and size to child node reg prop value failed\n", __func__);
		goto err;
	}

	if(fill_ranges_prop_value(offs, comm_info->srbuf.base, comm_info->srbuf.size, c_addr_cells, addr_cells, c_size_cells, &ranges) < 0) {
		printf("%s: fill srbuf base and size to range prop failed\n", __func__);
		goto err;
	}

	if(do_add_prop_to_fdt(fdt, 1, &ranges) < 0) {
		printf("%s: update ranges property to sipc node failed\n", __func__);
		goto err;
	}

	c_reg.offset = fdt_first_subnode(fdt, node_offset);
	if(c_reg.offset < 0) {
		printf("%s: the sipc child node not found\n", __func__);
		goto err;
	}

	if(do_add_prop_to_fdt(fdt, 0, &c_reg) < 0) {
		printf("%s: update reg property to sipc child node failed\n", __func__);
		goto err;
	}

	printf("update sipc node info done\n");
	print_fdt_node(fdt, node_offset);

	return 0;

err:
	print_fdt_node(fdt, node_offset);
	return -1;
}

static int do_parse_add_cproc_info(struct keyword *kwd, struct cp_mem_coupling_info *info, struct fdt_header *fdt)
{
	int i, index, len, state;
	int node_offset, c_node_offset;
	uint32_t offs, cp_base;
	uint32_t addr_cells = 2, size_cells = 1;
	uint32_t c_addr_cells = 2, c_size_cells = 1;
	char node_name[2*MAX_NAME_LEN] = {0};
	const char *value = NULL;
	struct property reg, ranges, ranges_bk, iram_dsize, iram_data;
	struct property c_reg, cproc_name;
	struct keyinfo *key;
	struct region_info *region = NULL;
	struct boot_info_conf *boot_info = NULL;

	if(NULL == kwd || NULL == info || NULL == fdt) {
		printf("%s: param error\n", __func__);
		goto err0;
	}

	boot_info = &info->boot_info;
	key = &kwd->key;
	node_offset = fdt_node_offset_by_prop_value(fdt, 0, key->name, key->value, strlen(key->value) + 1);
	if(node_offset < 0) {
		printf("%s: the cproc node do not support decoupling\n", __func__);
		//goto err0;
		return 1;
	}

	printf("start update cproc node info\n");
	print_fdt_node(fdt, node_offset);

	count_addr_size_cells(fdt, node_offset, &addr_cells, &size_cells);
	printf("%s: a_cells: %d, s_cells: %d\n", __func__, addr_cells, size_cells);

	property_init("reg", node_offset, &reg);
	property_init("ranges", node_offset, &ranges);
	property_init("sprd,iram-dsize", node_offset, &iram_dsize);
	property_init("sprd,iram-data", node_offset, &iram_data);

	if(fill_reg_prop_value(boot_info->cpmem.base, boot_info->cpmem.size, addr_cells, size_cells, &reg) < 0) {
		printf("%s: fill cpmem base and size to cproc reg prop failed\n", __func__);
		goto err0;
	}

	if(fill_reg_prop_value(boot_info->mdump.base, boot_info->mdump.size, addr_cells, size_cells, &reg) < 0) {
		printf("%s: fill mdump base and size to cproc reg prop failed\n", __func__);
	}

	if(fill_prop_value(num_t, &info->boot_info.bcode.count, sizeof(uint32_t), &iram_dsize) < 0 ) {
		printf("%s: fill boot code count to cproc iram_dsize prop failed\n", __func__);
		goto err0;
	}

	if(fill_prop_value(num_t, info->boot_info.bcode.code, info->boot_info.bcode.count * sizeof(uint32_t), &iram_data) < 0) {
		printf("%s: fill boot code count to cproc iram_data prop failed\n", __func__);
		goto err0;
	}

	cp_base = boot_info->cpmem.base;

	for(i = 0, region = boot_info->regions; i < MAX_REGION_NUM; i++, region++) {
		if(strlen(region->name) > 0) {
			c_node_offset = fdt_node_offset_by_prop_value(fdt, node_offset, "cproc,name", region->name, strlen(region->name) + 1);
			if(c_node_offset == -FDT_ERR_NOTFOUND) {
				sprintf(node_name, "%s@0x%x", region->name, region->base - cp_base);
				c_node_offset = fdt_add_subnode(fdt, node_offset, node_name);
				if(c_node_offset < 0) {
					printf("%s: create %s node failed\n", __func__, node_name);
					continue;
				}
			}

			count_addr_size_cells(fdt, c_node_offset, &c_addr_cells, &c_size_cells);

			property_init("ranges", node_offset, &ranges);
			offs = region->base - cp_base;
			if(fill_ranges_prop_value(offs, region->base, region->size, c_addr_cells, addr_cells, c_size_cells, &ranges) < 0) {
				printf("%s: fill ranges prop value failed, in %s node\n", __func__, node_name);
				goto err1;
			}
			if(do_add_prop_to_fdt(fdt, 1, &ranges) < 0) {
				printf("%s: add ranges property to cproc node failed\n", __func__);
				goto err1;
			}

			c_node_offset = fdt_subnode_offset(fdt, node_offset, node_name);
			if(c_node_offset < 0) {
				printf("%s: %s child node not found\n", __func__, node_name);
				continue;
			}

			property_init("reg", c_node_offset, &c_reg);
			property_init("cproc,name", c_node_offset, &cproc_name);

			if(fill_reg_prop_value(offs, region->size, c_addr_cells, c_size_cells, &c_reg) < 0) {
				printf("%s: fill reg prop value failed, in %s node\n", __func__, node_name);
				goto err1;
			}
			if(do_add_prop_to_fdt(fdt, 0, &c_reg) < 0) {
				printf("%s: add reg property failed, in %s node\n", __func__, node_name);
				goto err1;
			}

			if(fill_prop_value(str_t, region->name, sizeof(region->name), &cproc_name) < 0) {
				printf("%s: fill cproc_name prop value failed, in %s node\n", __func__, node_name);
				goto err1;
			}
			if(do_add_prop_to_fdt(fdt, 0, &cproc_name) < 0) {
				printf("%s: add cproc_name property failed, in %s node\n", __func__, node_name);
				goto err1;
			}

			continue;
err1:
			fdt_del_node(fdt, c_node_offset);
			continue;

		} else {
			break;
		}
	}

	if(do_add_prop_to_fdt(fdt, 0, &iram_dsize) < 0) {
		printf("%s: add iram_dsize property to cproc node failed\n", __func__);
	}

	if(do_add_prop_to_fdt(fdt, 0, &iram_data) < 0) {
		printf("%s: add iram_data property to cproc node failed\n", __func__);
	}

	if(do_add_prop_to_fdt(fdt, 1, &reg) < 0) {
		printf("%s: add reg property to cproc node failed\n", __func__);
		goto err0;
	}

	printf("update cproc node info done\n");
	print_fdt_node(fdt, node_offset);

	return 0;

err0:
	print_fdt_node(fdt, node_offset);
	return -1;
}

static int do_parse_add_rsvmem_info(struct keyword *kwd, struct cp_mem_coupling_info *info, struct fdt_header *fdt)
{
	int i, err;
	rsvmem_info_t *rsvmem;

	if(NULL == info || NULL == fdt)	{
		printf("%s: param error\n", __func__);
		return -1;
	}

	printf("start add cp rsvmem to fdt\n");
	print_fdt_rsvmem(fdt);

	for(rsvmem = info->rsvmem_info.rsvmem, i = 0; i < MAX_RSVMEM_NUM; i++, rsvmem++) {
		if(rsvmem->size == 0) {
			break;
		}

		printf("%s: rsvbase = 0x%p, rsvsize = 0x%p\n", __func__, rsvmem->base, rsvmem->size);
		err = fdt_add_mem_rsv(fdt, rsvmem->base, rsvmem->size);
		if (err < 0) {
			printf("%s: libfdt fdt_add_mem_rsv(): %s\n", __func__, fdt_strerror(err));
			return err;
		}
	}

	printf("add cp rsvmem to fdt done\n");
	print_fdt_rsvmem(fdt);

	return 0;
}

static int add_fdt_total_size(uint size, struct fdt_header *fdt)
{
	if(size <= 0 || NULL == fdt)	{
		printf("%s: param error\n", __func__);
		return -1;
	}

	if(fdt_totalsize(fdt) <= fdt_off_dt_strings(fdt) + fdt_size_dt_strings(fdt) + size) {
		fdt_set_totalsize(fdt, fdt_totalsize(fdt) + size);
	}

	return 0;
}

static int fixup_image_partition_name(const char *rname, struct partition_name_info *pinfo) {
	int num, index = 0;
	char *p = pinfo->partition;

	if(NULL == rname || NULL == pinfo) {
		printf("%s: param error\n", __func__);
		return -1;
	}

	strcpy(pinfo->partition, PARTITION_PREFIX);
	num = min(sizeof(pinfo->partition) - strlen(pinfo->partition) - 1, strlen(rname));
	strncat(pinfo->partition, rname, num);
	index = strlen(pinfo->partition) - strlen(NV_PARTITION_KEY);
	if(index >= 0) {
		p = pinfo->partition + index;
	}

	if(!strcmp(p, NV_PARTITION_KEY)) {
		strcpy(pinfo->partition_bk, pinfo->partition);
		num = min(sizeof(pinfo->partition) - strlen(pinfo->partition) - 1, strlen("1"));
		strncat(pinfo->partition, "1", num);
		num = min(sizeof(pinfo->partition_bk) - strlen(pinfo->partition_bk) - 1, strlen("2"));
		strncat(pinfo->partition_bk, "2", num);
	}

	return 0;
}

int get_coupling_info(const char *partition, struct cp_mem_coupling_info *info)
{
	int offs = 0;
	char buf[3*EMMC_SECTOR_SIZE] = {0};
#ifdef CONFIG_SECURE_BOOT
	char header[SEC_HEADER_MAX_SIZE] = {0};
#endif
	block_dev_desc_t *dev = NULL;

	if(NULL == partition || NULL == info){
		printf("%s: param error\n", __func__);
		return -1;
	}

	dev = get_dev("mmc", 0);
	if(NULL == dev) {
		printf("%s: get mmc dev failed\n", __func__);
		return -1;
	}

#ifdef CONFIG_SECURE_BOOT
	if(!_boot_partition_read(dev, partition, 0, EMMC_SECTOR_SIZE, (u8 *)header)) {
		printf("%s: read %s secure boot header error\n", __func__, partition);
		return -1;
	}
	offs = get_code_offset(header);
#endif

	if (!_boot_partition_read(dev, partition, offs, 3*EMMC_SECTOR_SIZE, (u8 *)buf)) {
		printf("%s: read %s mem coupling error\n", __func__, partition);
		return -1;
	}

	memcpy(info, buf, sizeof(struct cp_mem_coupling_info));

	return 0;
}

int fdt_fixup_cp_coupling_info(void *blob)
{
	int i, ret;
	unsigned long x = 0;
	char cmd_buf[64] = {0};
	struct fdt_header *fdt = NULL;
	struct partition_name_info modem_part;

	if(NULL == blob)	{
		printf("%s: param error\n", __func__);
		return -1;
	}

	fdt = (struct fdt_header *)blob;

	fixup_image_partition_name(DECOUPLING_INFO_PARTITION, &modem_part);

	if(g_info.magic != HEADER_INFO_MAGIC && get_coupling_info(modem_part.partition, &g_info) < 0) {
		printf("%s: read cp mem coupling info failed from %s\n", __func__, modem_part.partition);
		return -1;
	}

	printf("%s: magic = 0x%x, version = 0x%x\n", __func__, g_info.magic, g_info.version);
	if(HEADER_INFO_MAGIC != g_info.magic) {
		printf("%s: this version not support cp mem decoupling\n", __func__);
		return 1;
	}

	add_fdt_total_size(EXPAND_FDT_SIZE, fdt);

	for(i = 0; i < sizeof(g_keyword)/sizeof(struct keyword); i++) {
		if(g_keyword[i].do_parse_add) {
			ret = (g_keyword[i].do_parse_add)(&g_keyword[i].key, &g_info, fdt);
			if(ret < 0) {
				printf("%s: do parse add %s failed.\n", __func__, g_keyword[i].key.name);
				return -1;
			}
		}
	}

	return 0;
}

boot_image_required_t * get_cp_boot_table(void)
{
	int i, j;
	struct region_info *region = NULL;
	struct partition_name_info modem_part;

	fixup_image_partition_name(DECOUPLING_INFO_PARTITION, &modem_part);

	if(g_info.magic != HEADER_INFO_MAGIC && get_coupling_info(modem_part.partition, &g_info) < 0) {
		printf("%s: read cp mem coupling info failed from %s\n", __func__, modem_part.partition);
		return NULL;
	}

	printf("%s: magic: 0x%x, version: 0x%x\n", __func__, g_info.magic, g_info.version);
	if(HEADER_INFO_MAGIC != g_info.magic) {
		printf("%s: this version not support cp mem decoupling\n", __func__);
		return NULL;
	}

	for(i = 0, j = 0; i < MAX_REGION_NUM; i++) {
		region = &g_info.boot_info.regions[i];
		if(strlen(region->name) > 0) {
			if(region->lflag == re_load) {
				fixup_image_partition_name(region->name, &g_partition_info[j]);
				if(strlen(g_partition_info[j].partition) > 0) {
					s_boot_image_cp_table[j].partition = g_partition_info[j].partition;
				}
				if(strlen(g_partition_info[j].partition_bk) > 0) {
					s_boot_image_cp_table[j].bak_partition = g_partition_info[j].partition_bk;
				}

				s_boot_image_cp_table[j].size = region->size;
				s_boot_image_cp_table[j].mem_addr = region->base;

				if(s_boot_image_cp_table[j].bak_partition)
					printf("partition: %s, bak_partition: %s, size: 0x%x, mem_addr: 0x%x\n",
						s_boot_image_cp_table[j].partition, s_boot_image_cp_table[j].bak_partition,
						s_boot_image_cp_table[j].size, s_boot_image_cp_table[j].mem_addr);
				else
					printf("partition: %s, bak_partition: NULL, size: 0x%x, mem_addr: 0x%x\n",
						s_boot_image_cp_table[j].partition,
						s_boot_image_cp_table[j].size, s_boot_image_cp_table[j].mem_addr);
				j++;

			} else if(region->lflag == re_clear) {
				memset((char *)region->base, 0, region->size);
			}
		}
	}

#ifdef CP_MEM_DECOUP_DEBUG
	printf("\n--------------------------------------------------\n");
	for(i = 0; i < MAX_REGION_NUM; i++) {
		if(s_boot_image_cp_table[i].partition != NULL){
			printf("partition: %s, size: 0x%x, mem_addr: 0x%x\n",
							s_boot_image_cp_table[i].partition,
							s_boot_image_cp_table[i].size, s_boot_image_cp_table[i].mem_addr);
		}
	}
	printf("--------------------------------------------------\n");
#endif

	return s_boot_image_cp_table;
}

int get_boot_cp_code(uint32_t *count, uint32_t *code)
{
	int num;
	char partition[MAX_NAME_LEN] = {0};

	if(NULL == count || NULL == code) {
		printf("%s: param error\n", __func__);
		return -1;
	}

	strcpy(partition, PARTITION_PREFIX);
	num = min(sizeof(partition) - strlen(partition) - 1, strlen(DECOUPLING_INFO_PARTITION));
	strncat(partition, DECOUPLING_INFO_PARTITION, num);

	if(g_info.magic != HEADER_INFO_MAGIC && get_coupling_info(partition, &g_info) < 0) {
		printf("%s: read cp mem coupling info failed from %s\n", __func__, partition);
		return -1;
	}

	if(HEADER_INFO_MAGIC != g_info.magic) {
		printf("%s: this version not support cp mem decoupling\n", __func__);
		return -1;
	}

	*count = g_info.boot_info.bcode.count;
	memcpy(code, g_info.boot_info.bcode.code, *count * sizeof(uint32_t));

	return 0;
}

int get_sipc_smem_info(uint64_t *addr, uint32_t *size)
{
	int num;
	char partition[MAX_NAME_LEN] = {0};

	if(NULL == addr || NULL == size) {
		return -1;
	}

	strcpy(partition, PARTITION_PREFIX);
	num = min(sizeof(partition) - strlen(partition) - 1, strlen(DECOUPLING_INFO_PARTITION));
	strncat(partition, DECOUPLING_INFO_PARTITION, num);

	if(g_info.magic != HEADER_INFO_MAGIC && get_coupling_info(partition, &g_info) < 0) {
		printf("%s: read cp mem coupling info failed from %s\n", __func__, partition);
		return -1;
	}

	if(HEADER_INFO_MAGIC != g_info.magic) {
		printf("%s: this version not support cp mem decoupling\n", __func__);
		return -1;
	}

	*addr = g_info.comm_info.smem.base;
	*size = g_info.comm_info.smem.size;

	return 0;
}

