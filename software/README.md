# software notes 
---
## API deploy 


### project structure & system architecture 



We apply the **conceptual → logical → physical** model to guide the development of a distributed frequency monitoring system based on Software Defined Radio (SDR) sensors deployed across Colombia.

The system is designed as a distributed data acquisition and processing architecture:


![suggested structure](content/structure.png)


``` 

SDR Nodes (Colombia)
↓
Edge Processing Layer (pure C)
↓
Central API (Laravel/Node,Nest.js/FastAPI,flask,Django/Spring boot[java])
↓
Database + Storage
↓
Client Applications / Dashboard (Angular)

```



## conceptual design 


Develop an API capable of:

- Receiving frequency spectrum data from distributed SDR devices.
- Detecting signals within predefined frequency ranges.
- Storing spectral and metadata information.
- Exposing endpoints for querying frequency activity.
- Supporting analysis and monitoring services.

---
- **Software view:**  
High-level processing flow:

    - Capture signal → Convert to digital samples → Generate frequency spectrum → 
    - Filter by defined frequency range → Detect peaks / anomalies → Send data to API → store and expose results


Core logical agents (conceptual level):

- **SDR Agent:** Captures RF signals.
- **Processing Agent:** Performs FFT and filtering.
- **Detection Agent:** Identifies signals within target ranges.
- **Backend Agent:** Stores and exposes processed data.
- **Monitoring Agent:** Provides analysis endpoints.

At this stage, we define *what components exist* and *their responsibilities*, not the implementation details.

- **Database view:**  

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


  At this stage, we only define _what exists_ and _their relationships_, not the implementation.

## logical design 

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

    API Endpoints (example):
```

POST /api/sdr-data
GET /api/frequencies
GET /api/detections
GET /api/detections/{range_id}
GET /api/nodes
GET /api/alerts

```



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



---
## physical design ( implementation ) 


### software implementation 


### hardware implementation 


#### SRD nodes implementation



##### deployment 




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
