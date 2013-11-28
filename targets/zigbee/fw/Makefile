PROJECT_NAME = zigbee
#CONFIG_NAME = All_Sam7xEk_At91sam7x256_Rf231_Gcc
CONFIG_NAME = All_Sec_Sam7xEk_At91sam7x256_Rf231_Gcc

#CONFIG_NAME = All_Sam7xEk_At91sam7x256_Rf230B_Gcc
#CONFIG_NAME = All_Sec_Sam7xEk_At91sam7x256_Rf230B_Gcc
#CONFIG_NAME = All_Sam7xEk_At91sam7x256_Rf231_Gcc
#CONFIG_NAME = All_Sec_Sam7xEk_At91sam7x256_Rf231_Gcc
#CONFIG_NAME = All_Sam7xEk_At91sam7x256_Rf212_Gcc
#CONFIG_NAME = All_Sec_Sam7xEk_At91sam7x256_Rf212_Gcc
#CONFIG_NAME = All_Sam7xEk_At91sam7x256_Rf230B_Iar
#CONFIG_NAME = All_Sec_Sam7xEk_At91sam7x256_Rf230B_Iar
#CONFIG_NAME = All_Sam7xEk_At91sam7x256_Rf231_Iar
#CONFIG_NAME = All_Sec_Sam7xEk_At91sam7x256_Rf231_Iar
#CONFIG_NAME = All_Sam7xEk_At91sam7x256_Rf212_Iar
#CONFIG_NAME = All_Sec_Sam7xEk_At91sam7x256_Rf212_Iar

all:
	make -C makefiles/$(PROJECT_NAME) -f Makefile_$(CONFIG_NAME) all

clean:
	make -C makefiles/$(PROJECT_NAME) -f Makefile_$(CONFIG_NAME) clean
