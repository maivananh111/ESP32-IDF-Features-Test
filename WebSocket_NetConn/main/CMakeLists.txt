idf_component_register(SRCS "main.c"
							"WIFI/WIFI.c"
							"Server/Server.c"
							"WebSocket/WebSocket.c"
							"WebSocket_Server/WebSocket_Server.c"
	           INCLUDE_DIRS "."
	           				"WIFI"
                    		"Server"
                    		"WebSocket"
                    		"WebSocket_Server"
                EMBED_FILES HTML/test.css HTML/test.js HTML/root.html
)
