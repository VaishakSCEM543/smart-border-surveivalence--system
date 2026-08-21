# SBSS Literature Review Comparison (Papers 1-10)

---

## Paper 1

**No.:** 1

**Title:** Energy-Efficient Clustered Wireless Sensor Networks for Border Surveillance

**Authors:** Malathy et al.

**Year:** 2024

**Thought (comparison):**

Malathy and colleagues investigate energy-efficient clustered wireless sensor network (WSN) architectures for border surveillance, integrating ant colony optimization (ACO) algorithms for optimal routing performance. Their work addresses the fundamental challenge of balancing computational efficiency, energy consumption, and data security in resource-constrained environments requiring continuous operation. By organizing sensor nodes into hierarchical clusters with ACO-based routing and ECC cryptography, they achieve extended network lifetime and high packet delivery ratios validated through simulation.

In contrast, our proposed work implements direct WiFi bot-to-bot communication with real-time sensing and vision capabilities, avoiding complex multi-hop routing protocols entirely. Unlike their static clustered WSN approach using ACO for network-level routing optimization, we enable mobile, decentralized coordination through autonomous ESP32-CAM units. We emphasize local edge processing on ESP32-CAM for immediate autonomous responses to detected threats, eliminating routing delays and infrastructure complexity. While their system focuses on optimizing data transmission paths across static sensor networks, our design prioritizes on-device intelligence and context-aware decision-making without requiring centralized coordination or complex routing algorithms.

**Match Factor:** 35%

---

## Paper 2

**No.:** 2

**Title:** Integrated Multi-Sensor Data Fusion for Border Control

**Authors:** Essendorfer et al.

**Year:** 2024

**Thought (comparison):**

Essendorfer and colleagues present an integrated framework for border surveillance emphasizing heterogeneous sensor data fusion and transnational information sharing through the SOBCAH project. Their work addresses the complexity of modern border security where infrared cameras, electro-optical systems, radar, and sonar must operate cohesively across vast terrain. The core innovation lies in a unified data fusion architecture integrating disparate sensor streams into a coherent operational picture through probabilistic reasoning, with shared databases enabling real-time information exchange among border agencies across multiple countries.

We emphasize local, real-time decision-making at the edge using ESP32-CAM for immediate autonomous responses, eliminating reliance on centralized databases and cross-border data sharing protocols used in their centralized architecture. Unlike their approach requiring complex multi-sensor fusion infrastructure and transnational coordination, our system performs human-animal classification directly on-device, enabling independent operation without external server dependencies. While their work excels at large-scale interoperability and multi-agency coordination, our design prioritizes autonomous edge intelligence, privacy preservation through local processing, and simplified deployment suitable for remote border regions lacking complex infrastructure. Our ESP32-CAM units can operate completely offline, making decisions locally without requiring the extensive communication infrastructure their system depends upon.

**Match Factor:** 30%

---

## Paper 3

**No.:** 3

**Title:** Swarming Control for Unmanned Vehicles in Surveillance

**Authors:** Sauter et al.

**Year:** 2024

**Thought (comparison):**

Sauter and colleagues demonstrate swarming control algorithms for unmanned ground and aerial vehicles applied to surveillance and infrastructure protection. Their digital pheromone-based approach enables autonomous, adaptive coordination of heterogeneous sensor platforms, supporting complex behaviors including target tracking and multi-sensor fusion. The system mimics biological swarm intelligence where individual agents follow simple rules to achieve sophisticated collective behavior without centralized control, demonstrating robustness against individual agent failures and scalability to large deployments through real-world tests with UAVs, UGVs, and sensor networks.

We simplify swarm coordination into a practical, low-cost dual-bot system using direct WiFi communication, suitable for embedded deployment without the computational complexity of their digital-pheromone-based approach. Unlike their sophisticated swarm algorithms requiring complex coordination mechanisms and substantial computational resources, our ESP32-CAM based system implements straightforward bot-to-bot communication protocols optimized for resource-constrained edge devices. While their work demonstrates advanced multi-agent coordination with digital pheromones and complex behavioral algorithms, we prioritize deployment simplicity, cost-effectiveness, and immediate practicality for border surveillance. Our approach trades some coordination sophistication for dramatically reduced system complexity, lower unit costs, and easier field deployment, making it more accessible for resource-limited border security applications.

**Match Factor:** 40%

---

## Paper 4

**No.:** 4

**Title:** IoT-Based Smart Border Security with Machine Learning

**Authors:** Fatima et al.

**Year:** 2024

**Thought (comparison):**

Fatima and colleagues propose an IoT-based smart border security system utilizing machine learning for intruder detection, employing sensors, thermal imaging, and camera data analyzed by Faster R-CNN and EfficientDet models. Their architecture integrates edge computing for real-time processing with cloud-encrypted storage for long-term data retention and analysis, with automatic SMS alerts notifying security personnel upon threat detection. The system addresses practical deployment challenges in rugged terrains with limited manpower through multi-stage verification to mitigate false alarms and cloud integration for scalable remote monitoring.

We run lightweight CNN directly on ESP32-CAM through TinyML for offline operation with low latency, unlike their cloud-based ML models requiring constant connectivity and thermal imaging infrastructure. While their system depends on cloud resources for running computationally intensive models like Faster R-CNN and EfficientDet, our approach implements a custom lightweight neural network optimized specifically for ESP32-CAM's limited resources, enabling complete autonomous operation without internet connectivity. Unlike their requirement for thermal imaging cameras and cloud infrastructure, we achieve human-animal differentiation using standard RGB cameras with on-device inference, drastically reducing deployment costs and infrastructure requirements. Our system operates effectively in remote areas without cellular coverage or internet access, addressing a critical limitation in their cloud-dependent architecture.

**Match Factor:** 55%

---

## Paper 5

**No.:** 5

**Title:** Wireless Sensor Networks for Border Surveillance

**Authors:** Arjun et al.

**Year:** 2024

**Thought (comparison):**

Arjun and colleagues survey multiple wireless sensor network (WSN) techniques for border surveillance and intruder detection, highlighting hybrid systems integrating seismic, acoustic, infrared, and camera sensors to detect human intrusions in diverse border scenarios. The paper discusses multi-layered architectures and communication protocols critical for large-scale deployments, examining how different sensor modalities complement each other through multi-sensor fusion approaches. They analyze challenges including false alarms from wildlife, sensor coverage limitations, and energy constraints, proposing intelligent data aggregation and energy-efficient communication protocols to extend network lifetime.

We move beyond theoretical discussion by building and validating a working multi-sensor (PIR, ultrasonic) and vision-based prototype demonstrating practical implementation challenges and solutions. Unlike their comprehensive survey which primarily discusses theoretical architectures and proposed approaches, our work provides a functional, tested system addressing real-world deployment constraints. While they examine various sensor fusion methodologies abstractly, we implement practical sensor integration on resource-constrained ESP32-CAM hardware, demonstrating how lightweight CNN models can effectively process camera data alongside PIR and ultrasonic sensors for human-animal differentiation. Our contribution includes actual hardware implementation details, power consumption measurements, and field-tested performance metrics rather than simulation results or theoretical frameworks, filling the gap between their surveyed concepts and deployable border surveillance solutions.

**Match Factor:** 45%

---

## Paper 6

**No.:** 6

**Title:** Machine Learning Approaches Survey

**Authors:** Thomas et al.

**Year:** 2024

**Thought (comparison):**

Thomas and colleagues present a comprehensive survey on machine learning approaches, categorizing them into supervised, unsupervised, semi-supervised learning, and reinforcement learning methods. The paper explains key algorithms including decision trees, rule-based classifiers, naive Bayes, k-nearest neighbors, neural networks, and support vector machines, providing foundational understanding of their operational principles. They discuss applications across domains like pattern recognition, natural language processing, computer vision, and autonomous systems, offering insights directly relevant for AI-driven border surveillance and detection systems.

We apply ML concepts specifically by implementing CNN-based TinyML model on resource-constrained ESP32-CAM hardware for real-time visual intrusion detection, moving from theory to application. Unlike their broad theoretical survey covering multiple machine learning paradigms, our work focuses on practical deployment of convolutional neural networks optimized for edge devices with severe computational and memory constraints. While they discuss various algorithms academically, we address the engineering challenges of quantizing, optimizing, and deploying neural networks on microcontrollers with less than 8MB RAM. Our contribution demonstrates how theoretical ML concepts translate into functioning edge-AI surveillance systems, including model compression techniques, inference optimization strategies, and real-time performance under resource constraints—practical aspects their survey does not explore.

**Match Factor:** 30%

---

## Paper 7

**No.:** 7

**Title:** YOLO-Based Real-Time Border Security

**Authors:** Nahata et al.

**Year:** 2024

**Thought (comparison):**

Nahata and colleagues present a YOLO-based real-time border security surveillance system integrating deep learning with camera and UAV inputs to detect humans, vehicles, drones, and weapons. The paper highlights challenges in traditional border monitoring including limited visibility, vast coverage areas, and manual patrol inefficiencies. Their automated object detection approach using YOLO (You Only Look Once) architecture enables timely threat recognition with improved detection speed and accuracy, supporting integration with IoT devices for extended coverage and providing integrated alert mechanisms for rapid notification.

We replace YOLO with custom lightweight CNN optimized for ESP32-CAM's limited resources, trading minimal accuracy for drastically lower power consumption and cost, enabling edge processing. Unlike their approach requiring substantial computational resources to run YOLO models (typically on GPUs or high-end embedded platforms like Jetson boards), our lightweight CNN operates directly on ESP32-CAM's modest microcontroller. While YOLO-based systems excel in detection accuracy and multi-class object recognition, they require continuous power supply and expensive hardware unsuitable for distributed, solar-powered border deployments. Our design prioritizes energy efficiency, extended autonomous operation, and ultra-low unit cost (under $10 per node) over detection sophistication, making large-scale deployment economically feasible. This trade-off enables deploying hundreds of units for comprehensive coverage at the cost of a few YOLO-based systems.

**Match Factor:** 60%

---

## Paper 8

**No.:** 8

**Title:** Autonomous Robot Swarm for Border Intrusion

**Authors:** Mahjoub et al.

**Year:** 2024

**Thought (comparison):**

Mahjoub and colleagues develop an autonomous wireless sensor network employing a swarm of mobile robots for border intrusion detection, mimicking natural swarm behavior through distributed coordination algorithms. Their heterogeneous robot system collaboratively navigates complex terrains using YOLOv5 for object identification, with ROS (Robot Operating System) and Gazebo simulation environment utilized for system design, testing, and coordination protocol development. The study emphasizes cooperative navigation strategies, robust multi-robot communication protocols, and fault tolerance mechanisms enabling continued operation when subset of robots malfunction.

We adopt simpler, non-ROS architecture with low-power WiFi communication, prioritizing ease of deployment and cost-effectiveness over complex swarm behaviors using YOLOv5 and ROS. Unlike their sophisticated robotic platform requiring ROS middleware, complex simulation environments, and computationally intensive YOLOv5 models, our ESP32-CAM based system operates with minimal software dependencies and straightforward programming. While their approach demonstrates advanced multi-robot coordination and simulation-validated behaviors, it requires significant technical expertise for deployment and maintenance. Our design eliminates ROS complexity, replacing YOLOv5 with lightweight CNN suitable for microcontroller execution, and implements simple yet effective WiFi-based coordination that non-specialist personnel can deploy and maintain. This architectural simplification reduces development time, lowers technical barriers, and enables rapid field deployment without extensive simulation or complex coordination protocols.

**Match Factor:** 50%

---

## Paper 9

**No.:** 9

**Title:** Mini Seeker Robot for Border Surveillance

**Authors:** Reddy et al.

**Year:** 2024

**Thought (comparison):**

Reddy and colleagues propose the Mini Seeker Robot, an automated border surveillance platform equipped with ultrasonic sensors, facial recognition, and live video streaming functionalities. Controlled via Raspberry Pi microcontroller and integrated with OpenCV computer vision library, the robot detects obstacles autonomously and identifies individuals by matching facial data with pre-registered databases. The cost-effective solution operates autonomously following pre-programmed patrol routes or under remote manual control, with ultrasonic sensors enabling collision avoidance and facial recognition providing identity verification capabilities.

We substitute Raspberry Pi with ESP32-CAM paired with lightweight ML, achieving significant reduction in unit cost, power draw, and physical footprint for wider field deployment. Unlike their Raspberry Pi-based platform which consumes approximately 2.5-4W during operation and costs $35-75 per unit (depending on model), our ESP32-CAM system operates at under 1W and costs approximately $8-12 per unit. While Raspberry Pi provides greater computational flexibility for running OpenCV and complex facial recognition algorithms, it requires substantially more power, making solar-powered deployment challenging. Our approach trades facial recognition capability for human-animal classification—a more relevant distinction for border surveillance—while achieving dramatically longer battery life and lower deployment costs. This enables deploying 5-10 ESP32-CAM units for every Raspberry Pi robot, providing superior coverage density and system redundancy.

**Match Factor:** 65%

---

## Paper 10

**No.:** 10

**Title:** Helmet Detection and Number Plate Recognition

**Authors:** Sathe et al.

**Year:** 2024

**Thought (comparison):**

Sathe and colleagues focus on helmet detection and number plate recognition using deep learning models YOLOv5 and EasyOCR within traffic surveillance contexts. Their methodology involves bounding box overlap techniques to associate detected helmets with corresponding motorcyclists, effectively identifying helmet violation instances for traffic law enforcement. The study achieves high accuracy through transfer learning, leveraging pre-trained models and fine-tuning on domain-specific datasets, with comprehensive dataset augmentation including rotation, scaling, brightness adjustment, and synthetic variations improving model robustness.

We repurpose real-time object classification technology for identifying security threats (humans vs. animals) in border surveillance, adapting traffic monitoring techniques to security applications. Unlike their focus on traffic law enforcement through helmet and license plate detection, our system applies similar deep learning principles to border security contexts requiring human-animal differentiation. While their work demonstrates effective use of YOLOv5 and transfer learning for specific traffic scenarios, we adapt these methodologies to embedded edge devices with severe resource constraints. Their approach requires substantial computational resources for real-time video processing with YOLOv5 and OCR, whereas we implement simplified classification models optimized for ESP32-CAM's limited memory and processing capabilities. Our contribution demonstrates how traffic surveillance computer vision techniques can be efficiently adapted and compressed for deployment on ultra-low-cost edge devices in remote border monitoring applications.

**Match Factor:** 35%

---

---

## Paper 11

**No.:** 11

**Title:** Digital Border Surveillance System using YOLOv7

**Authors:** Noor et al.

**Year:** 2024

**Thought (comparison):**

Noor and colleagues propose a Digital Border Surveillance System (DBSS) using YOLOv7 for real-time object detection to monitor illegal migration and trafficking across borders. The system integrates IoT sensors including PIR and ultrasonic sensors with a custom dataset of 7347 images featuring uniformed personnel, civilians, and animals for comprehensive detection capability. Achieving 89% alarm success rate and 85% notification accuracy, the system implements intelligent threat-level differentiation with severe alarms for civilians, moderate alerts for unknown uniforms, and minor notifications for animals, with lateral movement filtering and centralized control room dashboard for video logging and real-time monitoring.

We perform human-animal differentiation locally on ESP32-CAM, enabling immediate threat assessment without server communication latency used in their YOLOv7 server-side processing. Unlike their centralized architecture where YOLOv7 detection occurs on powerful backend servers requiring continuous network connectivity, our lightweight CNN executes directly on-device, eliminating communication delays and enabling autonomous operation in network-denied environments. While their system achieves impressive accuracy with YOLOv7's sophisticated detection capabilities, it requires substantial infrastructure including centralized control rooms and reliable network connections. Our edge-based approach provides sub-second threat assessment and classification without external dependencies, critical for remote border areas lacking reliable connectivity. This architectural difference enables our system to make autonomous decisions immediately upon detection rather than waiting for server-side processing and response commands.

**Match Factor:** 65%

---

## Paper 12

**No.:** 12

**Title:** Thermal Image Pedestrian Detection

**Authors:** Kumar et al.

**Year:** 2024

**Thought (comparison):**

Kumar and colleagues develop a deep learning framework using YOLOv5 variants for pedestrian detection in thermal images, addressing limitations of RGB-based systems in challenging weather conditions including fog, rain, and nighttime operations. Trained on the UNIRI-TID thermal imaging dataset containing 7412 annotated images, YOLOv5-S achieves the highest mean Average Precision of 0.9908, outperforming tYOLOv3 across various thermal benchmarks while requiring less GPU and CPU resources. The study examines effects of layer freezing and data augmentation strategies, demonstrating particular relevance for border surveillance in deserts and mountainous regions where thermal imaging reliably detects human heat signatures regardless of lighting conditions.

We use standard RGB cameras with ultrasonic sensor fusion and lightweight ML, creating lower-cost solution optimized for daytime surveillance instead of thermal imaging for low-visibility conditions. Unlike their thermal imaging approach requiring specialized cameras costing $500-5000+ per unit, our ESP32-CAM uses standard RGB sensors available for under $10, dramatically reducing deployment costs. While thermal cameras excel in low-light and adverse weather conditions, they significantly increase system complexity and power requirements. Our design prioritizes cost-effectiveness and simplicity, acknowledging the trade-off of reduced nighttime performance for dramatically improved affordability and ease of deployment. For border surveillance applications with multiple detection points, deploying 50-100 RGB-based ESP32-CAM units provides superior daytime coverage density compared to a handful of thermal imaging systems, with supplementary infrared LEDs added to critical locations for nighttime enhancement at minimal cost.

**Match Factor:** 45%

---

## Paper 13

**No.:** 13

**Title:** YOLO-Based Surveillance with Automated Alerts

**Authors:** Taneja et al.

**Year:** 2024

**Thought (comparison):**

Taneja and colleagues present a YOLO-based real-time surveillance system detecting persons, vehicles, weapons, and drones, replacing labor-intensive patrols with automated threat identification and email alerts via Flask web framework integration. The methodology employs dataset annotation using Roboflow platform, enabling efficient labeling and preprocessing of training data. YOLO training provides single-stage detection with superior speed over two-stage methods like Faster R-CNN, with YOLOv8 achieving 94.9% drone detection accuracy and 70% overall mAP across multiple object classes. The system implements OAuth2-secured email notifications and discusses future expansion including IoT deployment for distributed sensor networks.

We implement direct bot-to-command-center WiFi alerts functioning in isolated networks without internet dependency, unlike their email alerts requiring internet connectivity. While their Flask-based system with OAuth2-secured email notifications demonstrates sophisticated cloud integration, it fundamentally requires internet connectivity and email infrastructure to function. Our ESP32-CAM units communicate directly via WiFi to local command stations using lightweight TCP/IP or MQTT protocols, enabling operation in remote border regions without cellular coverage or internet access. Unlike their reliance on cloud email services which introduce latency and infrastructure dependencies, our local alert system provides immediate notifications to nearby operators through dedicated WiFi networks or mesh configurations. This architectural choice prioritizes operational independence and zero-latency alerts over sophisticated cloud-based notification systems, making our solution viable in truly isolated deployment scenarios where internet connectivity is unavailable or unreliable.

**Match Factor:** 55%

---

## Paper 14

**No.:** 14

**Title:** IoT-Based Solar Charger for Field Deployments

**Authors:** Daou et al.

**Year:** 2024

**Thought (comparison):**

Daou and colleagues utilize Arduino UNO R4 WiFi and WebSocket communication to a Flutter mobile app for real-time photovoltaic and battery parameter visualization, creating an IoT-based MPPT solar charger. They achieve over 97% tracking efficiency with closely matching laboratory and app readings by combining perturb-and-observe MPPT algorithm, high-precision sensing, and low-latency WebSocket telemetry. This demonstrates how tightly coupled embedded control, sensing, and real-time IoT communication can be integrated into single low-cost platforms, with direct application to field-deployed, power-aware surveillance assets in remote border regions where solar charging extends operational lifetime without frequent battery replacement.

We adapt WebSocket-based telemetry approach for transmitting surveillance bot status, sensor readings, and alerts to command center, repurposing solar monitoring technique for security applications. Unlike their focus on solar charging optimization and energy parameter monitoring, our system extends the IoT telemetry concept to transmit surveillance-relevant data including detection events, camera status, battery levels, and GPS coordinates. While their work demonstrates efficient solar power management crucial for remote deployments, we integrate similar real-time communication protocols for operational monitoring rather than just power metrics. Our ESP32-CAM implementation benefits from their demonstrated WebSocket approach for low-latency, bidirectional communication, adapting it to transmit surveillance alerts and receive configuration updates. This technology transfer from renewable energy monitoring to security surveillance demonstrates how IoT telemetry frameworks can be repurposed across domains, with our contribution being the practical integration of power management monitoring alongside threat detection capabilities in a unified edge-intelligent platform.

**Match Factor:** 35%

---

## Paper 15

**No.:** 15

**Title:** 60 GHz CMOS-SOI Transceiver for IoT

**Authors:** Saponara et al.

**Year:** 2024

**Thought (comparison):**

Saponara and colleagues design essential components of a fully integrated 60 GHz CMOS-SOI transceiver for IoT nodes requiring multi-Gbps links over approximately 10 meters, targeting applications including vehicle-borne sensing systems, smart mobility, and surveillance. To meet ECMA-387-class link specifications at low power and compact area, they co-optimize wideband low-noise amplifiers, power amplifiers, and on-chip antennas, exploiting the 57-66 GHz unlicensed band. The millimeter-wave frequency enables high bandwidth for streaming multiple video feeds and sensor data simultaneously, though continuous power consumption must be carefully managed in battery-powered deployments, with short-range high-speed links ideal for vehicle-to-infrastructure communication.

We use standard WiFi offering longer range and lower power consumption suitable for surveillance applications, instead of high-speed, short-range 60 GHz transceivers for dense IoT networks. Unlike their millimeter-wave approach optimized for multi-Gbps data rates over 10-meter ranges, our 2.4 GHz WiFi implementation provides 50-100 meter range at modest data rates (1-10 Mbps) sufficient for surveillance video and sensor telemetry. While 60 GHz transceivers excel in bandwidth and interference immunity, they require line-of-sight conditions and consume significant power for RF operation. Our design prioritizes range, obstacle penetration, and power efficiency over bandwidth, recognizing that border surveillance applications require coverage area rather than ultra-high-speed links. Standard WiFi also benefits from ubiquitous infrastructure support, simplified debugging, and compatibility with existing network equipment, whereas 60 GHz systems require specialized hardware and expertise. This trade-off aligns with our emphasis on practical deployability and cost-effectiveness.

**Match Factor:** 25%

---

## Paper 16

**No.:** 16

**Title:** IoT-Powered Surveillance Robots

**Authors:** Geetha M et al.

**Year:** 2024

**Thought (comparison):**

Geetha M and colleagues present mobile robot platforms for surveillance integrating IoT connectivity with onboard sensing for remote monitoring applications. The IoT-Powered Smart Surveillance Sentinel Bot emphasizes basic autonomy in relatively straightforward environments with standard communication models, while the Model Spy Bot introduces Li-Fi (Light Fidelity) technology for low-RF-emission, light-based communication enabling short-range covert surveillance. Li-Fi's directional nature provides inherent security against interception compared to omnidirectional radio communications, though trading transmission range and robustness for reduced detectability and electromagnetic interference immunity. These studies demonstrate how low-cost robotics and alternative physical-layer links can enhance surveillance coverage in specific operational contexts.

We integrate separate robot concepts into unified WiFi-based dual-bot system with coordinated sensor-vision fusion for comprehensive situational awareness. Unlike their presentation of two distinct robot platforms with different communication technologies (standard RF and Li-Fi), our system implements a cohesive architecture where multiple ESP32-CAM units coordinate via standard WiFi for practical deployment. While Li-Fi offers interesting covert communication properties, its requirement for line-of-sight and limited range make it impractical for outdoor border surveillance where units may be separated by hundreds of meters with terrain obstacles. Our WiFi-based approach provides reliable omnidirectional communication suitable for distributed surveillance networks while achieving adequate security through WPA2/WPA3 encryption. Rather than exploring alternative physical layers, we focus on optimizing standard WiFi for multi-bot coordination, implementing efficient protocols for sharing detection events and coordinating patrol coverage across multiple autonomous units.

**Match Factor:** 40%

---

## Paper 17

**No.:** 17

**Title:** Navigation Interface for Multiple Robots

**Authors:** Sergio Saponara et al.

**Year:** 2024

**Thought (comparison):**

Sergio Saponara and colleagues develop a user-centric navigation interface and control framework enabling grouping and coordinated motion of multiple robots in shared environments. By providing interaction modalities and algorithms for grouping, tasking, and conflict management across multiple platforms, it reduces operator workload in multi-robot missions where single operators must manage numerous autonomous agents simultaneously. The interface allows operators to define robot groups, assign collective objectives, and monitor mission progress through intuitive visualization, with conflict management algorithms preventing collisions and deadlocks when multiple robots operate in proximity. Although primarily demonstrated in indoor or structured environments, the concepts directly apply to border-surveillance UGV swarms and patrol coordination.

We implement simplified, dedicated LCD-based command-center display focused on live border status monitoring rather than intricate navigation control interfaces. Unlike their sophisticated GUI framework designed for detailed robot navigation control, group management, and conflict resolution visualization, our command interface prioritizes essential surveillance information: detection events, bot status, battery levels, and coverage maps. While their system enables complex operator-directed navigation and task assignment, our design emphasizes autonomous operation with minimal operator intervention, reducing interface complexity to status monitoring and alert acknowledgment. Their approach suits applications requiring precise human-directed robot positioning, whereas border surveillance benefits more from autonomous patrol with exception-based human oversight. Our simplified LCD dashboard displays real-time detection feeds, threat classifications, and system health without navigation joysticks or complex tasking interfaces, reflecting our architectural philosophy of autonomous edge intelligence rather than centralized operator control.

**Match Factor:** 35%

---

## Paper 18

**No.:** 18

**Title:** AI-Based Human Detection for Storage Optimization

**Authors:** Alajrami et al.

**Year:** 2024

**Thought (comparison):**

Alajrami and colleagues design an AI-based desktop program using OpenCV and Haar-cascade classifiers under Linux to trigger recording only when human body or face is detected, optimizing storage utilization. They report approximately 99% body-detection and 96% face-detection accuracy on test sequences by switching from motion-only triggering to human-specific detection, reducing video storage by 27% compared to continuous recording while maintaining complete coverage of human activity. The system greatly reduces search effort and storage costs by eliminating hours of empty footage from archived recordings, providing practical storage optimization for large-scale surveillance deployments.

We employ deep-learning-based CNN classifier for improved accuracy and robustness to lighting variations, moving beyond traditional Haar-cascade classifiers. Unlike their use of Haar-cascade classifiers—a classical computer vision technique effective but sensitive to lighting conditions, pose variations, and partial occlusions—our CNN-based approach learns hierarchical feature representations providing superior generalization. While Haar-cascades require careful manual feature engineering and struggle with non-frontal views or varying illumination, our lightweight CNN trained on diverse datasets handles lighting variations, different poses, and cluttered backgrounds more robustly. Their reported 99% body detection likely represents favorable test conditions, whereas CNN models maintain high accuracy across diverse real-world scenarios including shadows, backlighting, and partial occlusions common in outdoor border surveillance. Additionally, our CNN simultaneously performs human-animal classification rather than just human detection, providing actionable intelligence beyond their binary detection approach. This enhanced classification capability enables context-aware responses critical for border security applications.

**Match Factor:** 50%

---

## Paper 19

**No.:** 19

**Title:** AI-Based Human Identification for Selective Recording

**Authors:** Eman Alajrami et al.

**Year:** 2024

**Thought (comparison):**

Eman Alajrami and colleagues discuss deployment factors for AI-based human identification including user acceptance, frame rate and resolution configuration, and integration with existing CCTV infrastructure. Evaluations with real organizations demonstrate that people find AI-assisted selective recording both useful and easy to use, suggesting successful integration into existing systems without requiring complete camera replacement. The study examines practical considerations including computational requirements, network bandwidth, storage architecture, and operator training needs, with user acceptance testing revealing that security personnel readily adopt AI-assisted systems when they demonstrably reduce workload without compromising security coverage.

We implement mobile robotic surveillance platform that actively patrols and scans areas rather than passively monitoring fixed camera feeds with AI triggering. Unlike their focus on retrofitting existing fixed CCTV infrastructure with AI-based selective recording, our system represents a fundamentally different paradigm: mobile autonomous surveillance rather than static camera upgrades. While their work addresses the practical challenge of enhancing legacy surveillance systems through software upgrades, we design purpose-built mobile platforms integrating sensing, intelligence, and mobility from the ground up. Their approach serves building security and facility monitoring where camera positions are predetermined, whereas border surveillance requires dynamic coverage of vast areas impossible to monitor with fixed cameras alone. Our mobile ESP32-CAM bots provide flexible deployment, adaptive coverage patterns, and the ability to investigate detected anomalies by moving closer, capabilities unavailable to fixed cameras regardless of AI enhancements. This represents a complementary rather than competing approach to surveillance system design.

**Match Factor:** 30%

---

## Paper 20

**No.:** 20

**Title:** Next-Generation Defense Monitoring with Digital Twin, Blockchain, and AI

**Authors:** Geetha et al.

**Year:** 2024

**Thought (comparison):**

Geetha and colleagues propose a next-generation defense monitoring system incorporating digital twin concepts, blockchain, IoT, and AI-based facial recognition for secure asset tracking and access control. The system provides immutable logs, automated responses via smart contracts, and high resilience to tampering by utilizing streaming face detection/recognition, GPS-based activity logging, and permissioned blockchain (Hyperledger Fabric). Experimental results in military-style facilities demonstrate strong resistance to simulated cyberattacks, approximately 98% face-recognition accuracy under favorable conditions, GPS localization within about 5 meters, and blockchain logging latency of 2-3 seconds. The blockchain architecture eliminates single points of failure with distributed ledger ensuring data integrity even if individual nodes are compromised.

We prioritize real-time intrusion detection and threat classification over individual identification and access management using blockchain and facial recognition. Unlike their sophisticated system combining digital twins, blockchain immutability, and facial recognition for facility access control and asset tracking, our design focuses on rapid perimeter intrusion detection with human-animal classification rather than individual identity verification. While their blockchain-based approach ensures tamper-proof audit trails and smart-contract-driven access control valuable for high-security facilities, it introduces computational overhead and latency unsuitable for time-critical border intrusion alerts. Our system trades comprehensive identity management and blockchain auditability for millisecond-level threat detection and classification, recognizing that border surveillance primarily requires knowing what crossed the border (human vs. animal) rather than who. Their 2-3 second blockchain latency, though impressive, exceeds acceptable response times for autonomous intrusion alerts where immediate action is critical.

**Match Factor:** 25%

---

**Status:** Papers 1-20 completed
**Remaining:** Papers 21-50
**Ready for:** Continuation to papers 21-30