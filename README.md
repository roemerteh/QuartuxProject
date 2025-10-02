# Monitor Service C++

This project is a monitor service in C++17 that:
- connect to MySql DB
- exposes a rest endpoint
- support dynamic configuration
- can run in Docker (pendding)

## description
This service launch two threads, one of them (server) is listening for petitions 'status' request 
and the other one (monitor) connect to a BD MySql and do queries every interval time and store the results in a log

## functionality
- Server 
launch a thread linked to a Rest Endpoint in the port 8080 (port in config/config.json) and wait for status petitions
for example:
```bash
curl http:://127.0.0.0:8080/status
```
expected results is the last status for process a query in the BD "Monitoring" for the table "metrics" like this: 
```json
{
	"last_check": "2025-07-08T10:23:00Z",
	"rows_processed": 3,
	"alerts": 1
}
```
also can see logs regarding each status request
```bash
tail -f /var/log/server_service.log
```
expected results:
```bash
{"timestamp":"2025-10-01T06:37:55","level":"INFO","msg":"start Server"}
{"timestamp":"2025-10-01T06:38:38","level":"INFO","msg":"status request received"}
{"timestamp":"2025-10-01T06:38:57","level":"INFO","msg":"stop Server waiting for thread"}
{"timestamp":"2025-10-01T06:42:10","level":"INFO","msg":"start�Server"}
```	

- Monitor 
launch a thread linked to a function in Monitor class which do queries every interval time (defined in config/config.json) 
and store the results in a log: monitor_service.log
example:
```bash
tail -f /var/log/monitor_service.log
```
expected results:
```bash
{"timestamp":"2025-10-01T16:37:25","level":"INFO","msg":"1 Temperature 42 2025-09-26 13:38:49"}
{"timestamp":"2025-10-01T16:37:25","level":"INFO","msg":"2 Pressure 60 2025-09-26 13:38:49"}
{"timestamp":"2025-10-01T16:37:25","level":"WARNING","msg":"threshold exceeded"}
{"timestamp":"2025-10-01T16:37:25","level":"INFO","msg":"3 Humidity 30 2025-09-26 13:38:49"}
{"timestamp":"2025-10-01T16:37:25","level":"INFO","msg":"processed�3�rows."}
```
### support dynamic configuration
This application support dynamic configuration
when this app catch a SIGHUP signal verify if /config/config.json has been modified if so determine if these changes affect to the server or to the monitor
or both and then restart the services loading and applying those changes
example:
make a change in /config/config.json for example: "rest_port"=8090 and saved
determine the PID of the service
```bash
ps -aux | grep QMonitor
```
once the pid is determined 
```bash
kill -HUP [PID]
```
expected result:
in terminal where server was launched is displayed:
```
{"level":"INFO","msg":"signal SIGHUP received"}
```
in /var/log/server_service.log can see those new lines:
```bash
{"timestamp":"2025-10-01T06:43:02","level":"INFO","msg":"stop Server waiting for thread"}
{"timestamp":"2025-10-01T16:35:44","level":"INFO","msg":"start�Server"}
```
it means that the server was restarted
and now the request can made to this new port
```bash
curl http:://127.0.0.0:8090/status
```
server and monitor only are restarted if the configuration affects them
	
## project structure
	QuartuxProject
	├── bin				# binary executable
	│   └── QMonitor
	├── config			# configuration file for DB and server
	│   └── config.json
	├── test			# test scenarios 
	│   └── TestScenarios.pdf	# some test scenarios with evidences are described in pdf file
	├── sql				 
	│   ├── init.sql	# initial configuration of DB
	│   ├── insert.sql	# inserting 10 new records 
	├── Doxyfile
	├── Makefile
	├── README.md
	├── src				# source code
	│   ├── Config.cpp
	│   ├── Config.h
	│   ├── Logger.cpp
	│   ├── Logger.h
	│   ├── main.cpp
	│   ├── Monitor.cpp
	│   ├── Monitor.h
	│   ├── RestServer.cpp
	│   └── RestServer.h
	└── utils			# external libraries
		├── httplib.h
	    └── json.hpp

## requirements
- Compilador C++17 (`g++`).
- [MySQL Connector/C++](https://dev.mysql.com/downloads/connector/cpp/).
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) (header only).	#included in "utils/"
- [nlohmann/json](https://github.com/nlohmann/json) (header only).		#included in "utils/"
		
## compilation
To generate binary file
```bash
make
```

## instalation
To create, change owner and give permissions to the log files
```bash
make install-log
```

## execution
```bash
./bin/QMonitor
```

## generate Docxygen
```bash
doxygen Doxyfile
```

## initial SQL configuration
This is a script available in sql/init.sql file 
```sql
CREATE DATABASE IF NOT EXISTS monitoring;
USE monitoring;

CREATE TABLE IF NOT EXISTS metrics (
	id INT AUTO_INCREMENT PRIMARY KEY,
	name VARCHAR(100),
	value INT,
	updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO metrics (name, value) VALUES
('Temperature', 42),
('Pressure', 60),
('Humidity', 30);
```

## test scenarios