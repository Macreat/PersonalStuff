# software notes 
---
## API deploy 
- **main goal:**  
This project seeks to develop an API capable of receiving frequency spectrum data, storing, processing it, and exposing the results to support analysis and monitoring services, designed with future enhancements in mind, particularly the integration of machine learning techniques to enable advanced automation processes.

### project structure & system architecture 



We apply the **conceptual → logical → physical** model to guide the development of a distributed frequency monitoring system based on Software Defined Radio (SDR) sensors deployed across Colombia.

![suggested structure](content/structure.png)

The system is designed as a distributed data acquisition and processing architecture:


``` 

SDR Nodes (Colombia) -> massive series time 
↓
Edge Processing Layer (pure C) -> protocol communication TCP/HTTP/JSON ; UDC 
↓
Central API (Laravel/Node,Nest.js/FastAPI,flask,Django/Spring boot[java]) -> API REST (js + angular) + ML 
↓
Database + Storage (PostgreSQL/MySQL) 
↓
Client Applications / Dashboard (Angular)

```


![suggested structure](content/basicUML.png)



## conceptual design (high-level idea)


Develop an API capable of:

- Receiving frequency spectrum data from distributed SDR devices.
- Detecting signals within predefined frequency ranges.
- Storing spectral and metadata information.
- Exposing endpoints for querying frequency activity.
- Supporting analysis and monitoring services.
- Implement machine learning for automatic detection and alert. 

---
- **Software view:**  
High-level processing flow:

    - Capture signal → Convert to digital samples → Generate frequency spectrum  
    → Filter by defined frequency range → Detect peaks / anomalies → Send data to API → store , process data and expose results


Core logical agents (conceptual level for general system):

- **SDR Agent:** Captures RF signals.
    - SDR produces a continuous stream of raw spectrum data. 
    - each measurment includes a timestamp, frequency, power level and associated node.
- **Communication agent:** here define a specific protocol of communication (TCP/UDP). 
- **Processing and storing Agent:** Receive data, Performs FFT and filtering. (sensors view)
- **Backend Agent:** Stores and exposes processed data.
- **Frontend agent:** responsible for data visualization and handling user requests.  

services to implement: 

- **Detection Agent:** Identifies signals within target ranges. (as anaylisis motor)
- **Monitoring Agent:** Provides analysis endpoints.


At this stage, we define *what components exist* and *their responsibilities*, not the implementation details.



- **Database view:**  
    
    Entity-Relationship model: 
    We need to use a datababe capable of handling massive time series, this implies a special indexing by time ranges. 

    Main entities:

    - `SDRNode`
    - `Location`
    - `FrequencyRange`
    - `SignalDetection`
    - `SpectrumSample`
    - `User`
    - `Alert`

    Relationships:

    - One `SDRNode` belongs to one `Location`.
    - One `SDRNode` generates many `SpectrumSample`.
    - One `SpectrumSample` may generate many `SignalDetection`.
    - One `FrequencyRange` may match many `SignalDetection`.
    - Users can subscribe to frequency alerts.


  At this stage, we only define _what entities exists_ and _their relationships_, for future ER diagram.


- **API view:**  

In this stage, we define how is the system exposed to the outside and how do its consumers interact with it.
This is the formal system interface layer, ie , defines the external and internal communication contracts of the system. 

    API interaction categories: 
    - `Ingestion API`
        Manage high frequency, low latency , structural validation and message versioning. 
    - `Monitoring`
        Filter by range, node or any other index 
    - `Alert`
    - `Adminstration`
        Register or update info, also manage users. 
    
    Capabilities: 

    - `register nodes`
    - `send data`
    - `consult data` 


    Communication model : (define as layers)
    - Transport: `TCP` -> `UDP` 
    - Protocol: `HTTP`
    - Serialization : `JSON`
    - schema : `UniversalDataCommunication` (UDC)
    ` All messages follow a standardized schema, with versioning and validation applied.`

    Architectural pattern: 

    - `REST endpoints`
    - `Event streaming chanel` (kafka-open source distributed event streaming platform for events/services)   
    - `WebSocket` for rial time.  


Here, we only define system capabilites. 
This means that, the api exposes ingestion, monitoring, alerting, and administrative functionalities while enforcing structured data exchange and versioning.

## logical design (formal model) 

redirect to a relationated data base management and their implementation 

- **Software view:**  
    Refinement into structured components:

    - `receive_sdr_data()`
    - `apply_fft()`
    - `filter_frequency_range(min_freq, max_freq)`
    - `detect_signal_power(threshold)`
    - `store_detection()`
    - `generate_alert()`
    - `get_frequency_activity()`


- **Database view (relational schema):**  
    Example relational structure:

    - `SDRNode(id, name, serial_number, location_id, status)`
    - `Location(id, city, latitude, longitude)`
    - `FrequencyRange(id, min_freq, max_freq, description)`
    - `SpectrumSample(id, node_id, timestamp, raw_data_path)`
    - `SignalDetection(id, sample_id, range_id, peak_frequency, power_level, timestamp)`
    - `User(id, name, email, role)`
    - `Alert(id, user_id, range_id, threshold, active)`

    Key elements:

    - Primary and foreign keys defined.
    - 1–N relationships between nodes and samples.
    - Many-to-many logic possible between users and monitored ranges.


  Here, we add attributes, primary/foreign keys, and define relations (1–N, N–M).

- **API view ():**  

given the software view, has some 
    API Endpoints (as example):
```

POST /api/sdr-data
GET /api/frequencies
GET /api/detections
GET /api/detections/{range_id}
GET /api/nodes
GET /api/alerts

```
---
## physical design ( implementation ) 


### software implementation 


### hardware implementation 


#### SRD nodes implementation



##### deployment and proofs. 




##  FINALIZATION CONDITIONS

![final prototype](finalPrototype.png)

The task is considered complete when:
- At least three SDR nodes are deployed and transmitting data.
- Frequency detection within predefined ranges is validated.
- API endpoints return accurate and structured data.
- Database integrity constraints are verified.
- Alert mechanism is functional.
- Documentation includes:
  - ER diagram.
  - Architecture diagram.
  - API endpoint specification.
  - Deployment instructions.
- Load testing confirms the system supports concurrent node transmissions.

---
