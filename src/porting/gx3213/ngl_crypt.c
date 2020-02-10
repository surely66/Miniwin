

#define TRNG_SIZE			32
#define KEY_LENGTH			16
#define KEY_INFO_NUM		32

typedef struct 
{
	DWORD dwWrappingKeyId;		//the previous key id
	DWORD ImportedKeyId;		//the current key id
	BYTE pWrappedKey[KEY_LENGTH];			//the current key
	UINT32 uiWrappedKeySize;	//the current key size
	aui_hdl dsc_hdl;			//dsc handle
	aui_hdl kl_hdl;				//kl handle
	tVA_CRYPT_Algorithm eAlgorithm;
	BOOL bEncryptNotDecrypt;	//**< Encrypt if true, decrypt if false */
	DWORD dwHandle;	//use for check
}tVA_cryptKeyInfo;

INT VA_CRYPT_ImportKey ( const BYTE * pWrappedKey , UINT32 uiWrappedKeySize , DWORD dwWrappingKeyId , DWORD * pImportedKeyId )
{
    VA_CRYPT_DEBUG("\n");
    tVA_cryptKeyInfo *pCryptKeyInfo;
    DWORD index;
    DWORD i;
    //VA_CRYPT_DUMP("pWrappedKey:", pWrappedKey, 16);
    /*Check Illegal values*/
    if(!pWrappedKey || !pImportedKeyId){
 	VA_CRYPT_ERROR("It is a illegal operations\n");
	return kVA_INVALID_PARAMETER;
    }

    pCryptKeyInfo = (tVA_cryptKeyInfo *)malloc(sizeof(tVA_cryptKeyInfo));
    memset(pCryptKeyInfo, 0, sizeof(tVA_cryptKeyInfo));
	
    if(dwWrappingKeyId == kVA_CRYPT_ROOT_KEY_LVL2_ID1)	/** Predefined ID of the chipset root key */
    {		
 	VA_CRYPT_DEBUG("kVA_CRYPT_ROOT_KEY_LVL2_ID1\n");
	
	for(index = 0; index < KEY_INFO_NUM; index++){
 	    if(!keyInfo[index])
		break;
	}
		
	pCryptKeyInfo->uiWrappedKeySize = uiWrappedKeySize;
	pCryptKeyInfo->dwWrappingKeyId = dwWrappingKeyId;	// the previous key id
	pCryptKeyInfo->ImportedKeyId = index;	// the current key id
	memcpy(pCryptKeyInfo->pWrappedKey, pWrappedKey, uiWrappedKeySize);
	
	(*pImportedKeyId) = index;
	keyInfo[index] = (void *)pCryptKeyInfo;	//save the current key info
	VA_CRYPT_DEBUG("ImportedKeyId: %d\n", *pImportedKeyId);

	return kVA_OK;
    }else if(dwWrappingKeyId == kVA_CRYPT_INVALID_ID){
	VA_CRYPT_ERROR("It is a invalid id\n");
	goto err2;
    } else if(dwWrappingKeyId == kVA_CRYPT_CLEAR_KEY){
	VA_CRYPT_DEBUG("kVA_CRYPT_CLEAR_KEY\n");
		
	for(index = 0; index < KEY_INFO_NUM; index++){
 	     if(!keyInfo[index])
		break;
	}
		
	pCryptKeyInfo->uiWrappedKeySize = uiWrappedKeySize;
	pCryptKeyInfo->dwWrappingKeyId = dwWrappingKeyId;	// the previous key id
	pCryptKeyInfo->ImportedKeyId = index;	// the current key id
	memcpy(pCryptKeyInfo->pWrappedKey, pWrappedKey, uiWrappedKeySize);
		
	(*pImportedKeyId) = index;
	keyInfo[index] = (void *)pCryptKeyInfo;	//save the current key info

	VA_CRYPT_DEBUG("ImportedKeyId: %d, keysize: %d\n", *pImportedKeyId, uiWrappedKeySize);
	return kVA_OK;
    } else{
	if(dwWrappingKeyId > KEY_INFO_NUM || dwWrappingKeyId < 0)
	     goto err2;
		
	/*If the previous key id has been released, return kVA_INVALID_PARAMETER*/
	if(!keyInfo[dwWrappingKeyId])
	    goto err2;
		
	for(index = 0; index < KEY_INFO_NUM; index++){
  	    if(keyInfo[index]){
		VA_CRYPT_DEBUG("dwWrappingKeyId: %d\n", dwWrappingKeyId);
		tVA_cryptKeyInfo *checkKeyInfo = (tVA_cryptKeyInfo *)keyInfo[index];
		if(checkKeyInfo->ImportedKeyId == dwWrappingKeyId){	/*key pass correct*/
		    for(i = 0; i < KEY_INFO_NUM; i++) {
	   	        if(!keyInfo[i]) {
		 	    pCryptKeyInfo->uiWrappedKeySize = uiWrappedKeySize;
			    pCryptKeyInfo->dwWrappingKeyId = dwWrappingKeyId;	// the previous key id
			    pCryptKeyInfo->ImportedKeyId = *pImportedKeyId;	// the current key id
			    memcpy(pCryptKeyInfo->pWrappedKey, pWrappedKey, uiWrappedKeySize);
			    (*pImportedKeyId) = i;
			    keyInfo[i] = (void *)pCryptKeyInfo;
			    VA_CRYPT_DEBUG("ImportedKeyId: %d\n", *pImportedKeyId);
			    return kVA_OK;
			} else if(i == (KEY_INFO_NUM - 1)) {
		 	    VA_CRYPT_ERROR("Resource Busy");
			    goto err2;
			}
	 	    }
		} else {
			continue;
		}
	  }
	  if(index == (KEY_INFO_NUM - 1)) {
		VA_CRYPT_ERROR("Resource Busy");
		goto err2;
	  }
      }
   }

err2:
   free(pCryptKeyInfo);
   pCryptKeyInfo = NULL;
   return kVA_INVALID_PARAMETER;
}
