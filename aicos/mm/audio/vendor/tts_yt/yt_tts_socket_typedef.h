#ifndef __YT_TTS_SOCKET_TYPEDEF_HEADER__
#define __YT_TTS_SOCKET_TYPEDEF_HEADER__



#if 0 //TERENCE---2021/01/21: disable these codes for UniSoc
	struct yt_hostent {
		char  *h_name;      /* Official name of the host. */
		char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
							   terminated by a null pointer. */
		int    h_addrtype;  /* Address type. */
		int    h_length;    /* The length, in bytes, of the address. */
		char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
							   network byte order) for the host, terminated by a null pointer. */
	#define h_addr h_addr_list[0] /* for backward compatibility */
	};




	struct yt_in_addr //4 bytes
	{
		unsigned int s_addr_new; 
	};




	struct yt_sockaddr_in //16 bytes
	{
	  unsigned char sin_len;
	  unsigned char sin_family;
	  unsigned short sin_port;
	  struct yt_in_addr sin_addr;
	  char sin_zero[8];
	};





	struct yt_sockaddr //16 bytes
	{
	  unsigned char sa_len;
	  unsigned char sa_family;
	  char sa_data[14];
	};



	typedef struct yt_sockaddr*  ai_addr;

	typedef int yt_socklen_t;

#endif



	
	struct in_addr_yt 
	{
	  unsigned int s_addr;
	};

	struct yt_sockaddr_in //16 bytes
	{
	  unsigned char sin_len;
	  unsigned char sin_family;
	  unsigned short sin_port;
	  struct in_addr_yt sin_addr;
	  char sin_zero[8];
	};


	struct yt_sockaddr //16 bytes
	{
	  unsigned char sa_len;
	  unsigned char sa_family;
	  char sa_data[14];
	};


	typedef int yt_socklen_t;



	//for DNS : gethostbyname(...)
	struct yt_hostent {
		char  *h_name;      /* Official name of the host. */
		char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
							   terminated by a null pointer. */
		int    h_addrtype;  /* Address type. */
		int    h_length;    /* The length, in bytes, of the address. */
		char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
							   network byte order) for the host, terminated by a null pointer. */
		#define h_addr h_addr_list[0] /* for backward compatibility */
	};





#endif
