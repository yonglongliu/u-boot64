#include "sansa_secure_boot.h"

void flush_dcache_area(void *va,size_t len)
{
	flush_dcache_range(va, (char *)va + len -1);
}

void invalidate_dcache_area(void *va,size_t len)
{
	invalidate_dcache_range(va, (char *)va + len -1);
}

static unsigned int SB_MemoryRead(uint64_t offset,uint8_t *memDst,uint32_t sizeToRead,void *context)
{
	uint8_t *load_addr = NULL;
	uint8_t *base_addr = NULL;

	base_addr = (uint8_t *)context;
	load_addr = offset + base_addr;
	if (load_addr != memDst) {
		memcpy(memDst,load_addr,sizeToRead);
	}

	return 0;
}

static DxError_t DX_SB_CertPkgInfoInit(DxSbCertInfo_t *certpkginfo, void *pubkeyhash)
{
	memcpy(certpkginfo->pubKeyHash, (uint8_t *)pubkeyhash, sizeof(HASH_Result_t));
	certpkginfo->initDataFlag = 1;
	return 0;
}

unsigned int DX_SecureBootVerify(void *imgaddr, void *pubkeyhash)
{
	uint8_t scheme_level = 0;
	uint32_t ret;
	DxSbCertInfo_t *p_certPkgInfo = &certPkgInfo;

	fmpreambleheader *fm_header = (fmpreambleheader *)((uint8_t *)imgaddr +((sys_img_header *)imgaddr)->mImgSize + sizeof(sys_img_header));

	if ((fm_header->certa_size <= 0)
		&&(fm_header->certb_size <= 0)
		&&(fm_header->certcnt_size <= 0)) {
		return 1;
	}

	if ((fm_header->certa_size > 0)
		&&(fm_header->certb_size > 0)
		&&(fm_header->certcnt_size > 0)) {
		scheme_level = 3;
	} else if ((fm_header->certa_size > 0)
		&&(fm_header->certb_size == 0)
		&&(fm_header->certcnt_size > 0)) {
		scheme_level = 2;
	} else if ((fm_header->certa_size == 0)
		&&(fm_header->certb_size == 0)
		&&(fm_header->certcnt_size > 0)) {
		scheme_level = 1;
	}

	switch (scheme_level) {
		case 3:
			ret = DX_SB_CertChainVerificationInit(p_certPkgInfo);
			if (ret != DX_OK) {
				return ret;
			}

			if (pubkeyhash != NULL) {
				ret = DX_SB_CertPkgInfoInit(p_certPkgInfo,pubkeyhash);
				if (ret != DX_OK) {
					return ret;
				}
			}

			ret = DX_SB_CertVerifySingle(SB_MemoryRead,imgaddr,
										CRYPTOCELL_SECURE_BASE,
										fm_header->certa_offset,
										p_certPkgInfo,
										tmp_workspace_buf,
										DX_DOUBLE_BUFFER_MAX_SIZE_IN_BYTES);
			if (ret != DX_OK) {
				return ret;
			}

			ret = DX_SB_CertVerifySingle(SB_MemoryRead,imgaddr,
										CRYPTOCELL_SECURE_BASE,
										fm_header->certb_offset,
										p_certPkgInfo,
										tmp_workspace_buf,
										DX_DOUBLE_BUFFER_MAX_SIZE_IN_BYTES);
			if (ret != DX_OK) {
				return ret;
			}

			ret = DX_SB_CertVerifySingle(SB_MemoryRead,imgaddr,
										CRYPTOCELL_SECURE_BASE,
										fm_header->certcnt_offset,
										p_certPkgInfo,
										tmp_workspace_buf,
										DX_DOUBLE_BUFFER_MAX_SIZE_IN_BYTES);
			if (ret != DX_OK) {
				return ret;
			}
			break;
		case 2:
			ret = DX_SB_CertChainVerificationInit(p_certPkgInfo);
			if (ret != DX_OK) {
				return ret;
			}

			ret = DX_SB_CertPkgInfoInit(p_certPkgInfo,pubkeyhash);
			if (ret != DX_OK) {
				return ret;
			}

			ret = DX_SB_CertVerifySingle(SB_MemoryRead,imgaddr,
										CRYPTOCELL_SECURE_BASE,
										fm_header->certa_offset,
										p_certPkgInfo,
										tmp_workspace_buf,
										DX_DOUBLE_BUFFER_MAX_SIZE_IN_BYTES);
			if (ret != DX_OK) {
				return ret;
			}

			ret = DX_SB_CertVerifySingle(SB_MemoryRead,imgaddr,
										CRYPTOCELL_SECURE_BASE,
										fm_header->certcnt_offset,
										p_certPkgInfo,
										tmp_workspace_buf,
										DX_DOUBLE_BUFFER_MAX_SIZE_IN_BYTES);
			if (ret != DX_OK) {
				return ret;
			}
			break;
		case 1:
			ret = DX_SB_CertChainVerificationInit(p_certPkgInfo);
			if (ret != DX_OK) {
				return ret;
			}

			ret = DX_SB_CertPkgInfoInit(p_certPkgInfo,pubkeyhash);
			if (ret != DX_OK) {
				return ret;
			}

			ret = DX_SB_CertVerifySingle(SB_MemoryRead,imgaddr,
										CRYPTOCELL_SECURE_BASE,
										fm_header->certcnt_offset,
										p_certPkgInfo,
										tmp_workspace_buf,
										DX_DOUBLE_BUFFER_MAX_SIZE_IN_BYTES);
			if (ret != DX_OK) {
				return ret;
			}
			break;
		default:
			break;
	}

	return ret;
}

void sansa_get_hash_key(uint8_t *root_addr,uint8_t *hash_key)
{
	uint8_t *pKeyCert = NULL;
	sys_img_header *img_hdr = (sys_img_header *)root_addr;
	fmpreambleheader *fm_header = (fmpreambleheader *)((uint8_t*)root_addr + img_hdr->mImgSize + sizeof(sys_img_header));
	if ((fm_header->certa_size > 0) && (fm_header->certb_size > 0)) {
		pKeyCert = (uint8_t*)root_addr + fm_header->certb_offset;
	} else if ((fm_header->certa_size > 0) && (fm_header->certb_size == 0)) {
		pKeyCert = (uint8_t*)root_addr + fm_header->certa_offset;
	}

	uint32_t offset = sizeof(DxSbCertHeader_t) + SB_RSA_MOD_SIZE_IN_WORDS*sizeof(uint32_t) + SB_RSA_HW_PKI_PKA_BARRETT_MOD_TAG_SIZE_IN_WORDS*sizeof(uint32_t) + sizeof(DxSbSwVersion_t);
	memcpy(hash_key,pKeyCert+offset, HASH_BYTE_LEN);
}

void sansa_get_nptr(uint8_t *root_addr, uint8_t *nptr)
{
	uint8_t *pKeyCert = NULL;
	sys_img_header *img_hdr = (sys_img_header *)root_addr;
	fmpreambleheader *fm_header = (fmpreambleheader *)((uint8_t*)root_addr + img_hdr->mImgSize + sizeof(sys_img_header));
	pKeyCert = (uint8_t*)root_addr + fm_header->certa_offset;

	uint32_t offset = sizeof(DxSbCertHeader_t);
	memcpy(nptr,pKeyCert+offset, N_REVNP_SIZE_IN_BYTES);
}

void sansa_secure_check(void *rootaddr, void *imgaddr)
{

	uint8_t *pHashKey = NULL;
	uint8_t hash_key[HASH_BYTE_LEN];
	if (rootaddr != NULL) {
		memset(hash_key, 0, HASH_BYTE_LEN);
		sansa_get_hash_key(rootaddr,hash_key);
		pHashKey = hash_key;
	}

	if (DX_OK != DX_SecureBootVerify((uint8_t*)imgaddr,(uint8_t*)pHashKey)) {
		while(1);
	}
}


void sansa_dl_check(void *rootaddr, void *imgaddr)
{
	uint8_t N_ptr[N_REVNP_SIZE_IN_BYTES] = {0};
	uint8_t pubkhash[HASH_BYTE_LEN] = {0};
	uint32_t err = 1;

	sansa_get_nptr((uint8_t*)rootaddr, N_ptr);
	err = DX_SB_CalcPublicKeyHASH(CRYPTOCELL_SECURE_BASE, N_ptr, pubkhash);

	if ((DX_OK != err) || (DX_OK != DX_SecureBootVerify((uint8_t*)imgaddr,pubkhash))) {
		while(1);
	}
}

void sansa_cpy_spl_to_iram(void *rootaddr, void *imgaddr)
{
	uint32_t len = 0;
	fmpreambleheader *fm_header = (fmpreambleheader *)((uint8_t *)imgaddr +((sys_img_header *)imgaddr)->mImgSize + sizeof(sys_img_header));
	len = sizeof(sys_img_header) + ((sys_img_header *)imgaddr)->mImgSize + sizeof(fmpreambleheader) + fm_header->certa_size + fm_header->certb_size + fm_header->certcnt_size + fm_header->cert_dbg_second_size;
	if ((uint8_t *)rootaddr != (uint8_t *)imgaddr) {
		memcpy((uint8_t *)rootaddr,(uint8_t *)imgaddr,len);
	}
}

void sansa_dl_secure_verify(uint8_t *name, uint8_t *header, uint8_t *code)
{
	if (name) {
		if ((strcmp("splloader",name) == 0) || (strcmp("fdl1",name) == 0)) {
			sansa_secure_check(IRAM_BEGIN, header);
		} else if (strcmp("fdl2",name) == 0) {
			sansa_secure_check((CONFIG_SYS_TEXT_BASE - 0x200), header);
		} else if (strcmp("splloader0",name) == 0) {
			//sansa_dl_check(IRAM_BEGIN, header);
			sansa_secure_check(NULL, header);
			sansa_cpy_spl_to_iram(IRAM_BEGIN, header);
		} else if (strcmp("uboot",name) == 0) {
			sansa_secure_check((CONFIG_SYS_TEXT_BASE - 0x200), header);
		}
	}
}
