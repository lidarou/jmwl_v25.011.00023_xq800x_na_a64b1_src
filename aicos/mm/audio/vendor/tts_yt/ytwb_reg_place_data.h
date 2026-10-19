#ifndef __YTWB_REG_PLACE_DATA_HEADER__
#define __YTWB_REG_PLACE_DATA_HEADER__

#define __YTWB_REG_PLACE_LOOKUP_SUPPORT__

char* RegPlace_GetData(unsigned int *pDataSize);
char* RegPlace_GetIndex(unsigned int *pIndexSize);

char* RegPlace_GetPlaceSet(unsigned int *pPlaceSetSize);
char* RegPlace_GetOperatorSet(unsigned int *pOPerSetSize);

char* RegPlace_GetAddedFixedPlace(unsigned int *pFixedPlaceSize);


#endif
