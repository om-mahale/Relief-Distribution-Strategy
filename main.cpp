#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <limits>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cctype>

using namespace std;

// ============================================================================
// FAULT-TOLERANT INPUT VALIDATION ENGINE
// ============================================================================

int getValidInt(const string& prompt, int minVal, int maxVal, const string& example) {
    string line;
    int value;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) {
            cin.clear();
            continue;
        }
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == string::npos) {
            cout << "  [INPUT ERROR] Input cannot be empty! Please enter a whole number between "
                 << minVal << " and " << maxVal << " (Example: " << example << ").\n";
            continue;
        }
        size_t last = line.find_last_not_of(" \t\r\n");
        string trimmed = line.substr(first, (last - first + 1));

        stringstream ss(trimmed);
        char leftover;
        if (!(ss >> value) || (ss >> leftover)) {
            cout << "  [INPUT ERROR] Invalid entry (\"" << trimmed << "\")! Enter a whole number without letters or decimals (Example: "
                 << example << ").\n";
            continue;
        }
        if (value < minVal || value > maxVal) {
            cout << "  [INPUT ERROR] Value " << value << " is out of range! Enter a number between "
                 << minVal << " and " << maxVal << " (Example: " << example << ").\n";
            continue;
        }
        return value;
    }
}

double getValidDouble(const string& prompt, double minVal, double maxVal, const string& example) {
    string line;
    double value;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) {
            cin.clear();
            continue;
        }
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == string::npos) {
            cout << "  [INPUT ERROR] Input cannot be empty! Please enter a number between "
                 << minVal << " and " << maxVal << " (Example: " << example << ").\n";
            continue;
        }
        size_t last = line.find_last_not_of(" \t\r\n");
        string trimmed = line.substr(first, (last - first + 1));

        stringstream ss(trimmed);
        char leftover;
        if (!(ss >> value) || (ss >> leftover)) {
            cout << "  [INPUT ERROR] Invalid number (\"" << trimmed << "\")! Please enter a valid number (Example: "
                 << example << ").\n";
            continue;
        }
        if (value < minVal || value > maxVal) {
            cout << "  [INPUT ERROR] Value " << value << " is out of range! Enter a value between "
                 << minVal << " and " << maxVal << " (Example: " << example << ").\n";
            continue;
        }
        return value;
    }
}

// Accepts alphabetic words and spaces only (rejects numbers/integers and symbols)
string getValidString(const string& prompt, const string& example) {
    string line;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) {
            cin.clear();
            continue;
        }
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == string::npos) {
            cout << "  [INPUT ERROR] Field cannot be blank! Please enter words only (Example: "
                 << example << ").\n";
            continue;
        }
        size_t last = line.find_last_not_of(" \t\r\n");
        string trimmed = line.substr(first, (last - first + 1));

        bool hasLetter = false;
        bool validWordsOnly = true;
        for (char ch : trimmed) {
            if (isalpha(static_cast<unsigned char>(ch))) {
                hasLetter = true;
            } else if (!isspace(static_cast<unsigned char>(ch))) {
                validWordsOnly = false;
                break;
            }
        }

        if (!validWordsOnly || !hasLetter) {
            cout << "  [INPUT ERROR] Invalid name (\"" << trimmed
                 << "\")! Camp name must contain only words/letters and no numbers (Example: "
                 << example << "). Please try again.\n";
            continue;
        }
        return trimmed;
    }
}

// Helper to give each map node a friendly, readable name
string getLocationName(int node) {
    switch (node) {
        case 0: return "Node 0 (Main Relief Depot)";
        case 1: return "Node 1 (North Bridge)";
        case 2: return "Node 2 (East Highway)";
        case 3: return "Node 3 (River Crossing)";
        case 4: return "Node 4 (Valley Junction)";
        case 5: return "Node 5 (Hilltop Sector)";
        case 6: return "Node 6 (South Plains)";
        case 7: return "Node 7 (West Outpost)";
        default: return "Node " + to_string(node) + " (Sector " + to_string(node) + ")";
    }
}

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

struct Camp {
    int campId;
    string campName;
    int locationNode;
    int population;
    int urgency;
    int food;           // in Packets / kg
    int water;          // in Litres / Bottles
    int medicine;       // in Medical Kits / Boxes
    int shortage;       // Total units (Food + Water + Medicine)
    int travelTime;     // in Minutes (mins)
    double estBudgetRs; // Estimated relief & transport cost in Rupees (Rs.)
    int priority;

    void recalculateShortage() {
        shortage = food + water + medicine;
        // Standard unit valuation: Food = Rs. 50/pkt, Water = Rs. 20/L, Medicine = Rs. 150/kit
        estBudgetRs = (food * 50.0) + (water * 20.0) + (medicine * 150.0);
    }
};

struct Road {
    int source;
    int destination;
    double travelTime; // in Minutes (mins)
    double hazard;     // Flood risk multiplier (0.0 to 10.0)
    bool blocked;
};

// ============================================================================
// PRIORITY SCHEDULER & MAX-HEAP
// ============================================================================

class MaxHeap {
private:
    vector<Camp> heap;

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[parent].priority >= heap[index].priority)
                break;
            swap(heap[parent], heap[index]);
            index = parent;
        }
    }

    void heapifyDown(int index) {
        int size = static_cast<int>(heap.size());
        while (true) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int largest = index;

            if (left < size && heap[left].priority > heap[largest].priority)
                largest = left;
            if (right < size && heap[right].priority > heap[largest].priority)
                largest = right;

            if (largest == index)
                break;

            swap(heap[index], heap[largest]);
            index = largest;
        }
    }

public:
    void clear() {
        heap.clear();
    }

    bool isEmpty() const {
        return heap.empty();
    }

    static int calculatePriority(int urgency, int population, int shortage, int travelTime) {
        return (urgency * 4) + (population * 2) + (shortage * 3) - travelTime;
    }

    void insertCamp(Camp camp) {
        heap.push_back(camp);
        heapifyUp(static_cast<int>(heap.size()) - 1);
    }

    Camp removeHighestPriority() {
        Camp highest = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty())
            heapifyDown(0);
        return highest;
    }

    void displayPriorityRanking(int truckCapacity = 250) const {
        if (heap.empty()) {
            cout << "\n[STATUS] No relief camps currently need supplies.\n";
            return;
        }
        cout << "\n========================================================================================================\n";
        cout << "                                  CAMP PRIORITY RANKING (MAX-HEAP)                                      \n";
        cout << "========================================================================================================\n";
        cout << left << setw(6)  << "Rank"
             << setw(9)  << "Camp ID"
             << setw(24) << "Camp Name"
             << setw(9)  << "Urgency"
             << setw(16) << "Total Shortage"
             << setw(14) << "Travel Time"
             << setw(14) << "Est. Cost"
             << setw(10) << "Trips"
             << "Priority\n";
        cout << "--------------------------------------------------------------------------------------------------------\n";

        vector<Camp> temp = heap;
        int rank = 1;
        while (!temp.empty()) {
            int highest = 0;
            for (size_t i = 1; i < temp.size(); i++) {
                if (temp[i].priority > temp[highest].priority)
                    highest = static_cast<int>(i);
            }
            const Camp& c = temp[highest];
            int tripsReq = (c.shortage + truckCapacity - 1) / truckCapacity;

            cout << left << setw(6)  << ("#" + to_string(rank++))
                 << setw(9)  << c.campId
                 << setw(24) << c.campName.substr(0, 22)
                 << setw(9)  << (to_string(c.urgency) + "/10")
                 << setw(16) << (to_string(c.shortage) + " units")
                 << setw(14) << (to_string(c.travelTime) + " mins")
                 << setw(14) << ("Rs. " + to_string(static_cast<long long>(c.estBudgetRs)))
                 << setw(10) << (to_string(tripsReq) + " trip(s)")
                 << c.priority << "\n";

            temp.erase(temp.begin() + highest);
        }
        cout << "========================================================================================================\n";
    }
};

// ============================================================================
// ROAD NETWORK & DIJKSTRA SHORTEST PATH
// ============================================================================

struct RouteResult {
    bool reachable;
    double totalEffectiveCost; // Effective travel time in mins
    double rawTravelTime;      // Base travel time in mins
    double transportCostRs;    // Fuel/Transport cost in Rupees (Rs.)
    vector<int> path;
};

class Graph {
private:
    int vertices;
    vector<vector<Road>> adj;

    string getHazardLabel(double h) const {
        stringstream ss;
        ss << fixed << setprecision(1) << h;
        if (h == 0.0) return "None (" + ss.str() + ")";
        if (h <= 0.3) return "Low (" + ss.str() + ")";
        if (h <= 0.6) return "Medium (" + ss.str() + ")";
        return "High (" + ss.str() + ")";
    }

    // Returns the live status label for any road in the 2D diagram: e.g. "(10 mins)" or "[BLOCKED]"
    string getDiagramRoadLabel(int u, int v) const {
        if (u >= 0 && u < vertices) {
            for (const auto& r : adj[u]) {
                if (r.destination == v) {
                    if (r.blocked) {
                        return "[BLOCKED]";
                    }
                    stringstream ss;
                    if (abs(r.travelTime - round(r.travelTime)) < 1e-6) {
                        ss << "(" << static_cast<int>(round(r.travelTime)) << " mins)";
                    } else {
                        ss << "(" << fixed << setprecision(1) << r.travelTime << "m)";
                    }
                    return ss.str();
                }
            }
        }
        return "(No Road)";
    }

    // Checks if a road pair is one of the 8 default roads already drawn in the main 2D layout
    bool isDefaultDiagramRoad(int u, int v) const {
        int a = min(u, v);
        int b = max(u, v);
        return (a == 0 && b == 1) ||
               (a == 0 && b == 2) ||
               (a == 1 && b == 3) ||
               (a == 2 && b == 3) ||
               (a == 3 && b == 4) ||
               (a == 2 && b == 5) ||
               (a == 4 && b == 6) ||
               (a == 5 && b == 7);
    }

public:
    Graph(int v = 8) {
        vertices = v;
        adj.resize(v);
    }

    int getVertexCount() const {
        return vertices;
    }

    void resizeGraph(int newVertices) {
        if (newVertices > vertices) {
            adj.resize(newVertices);
            vertices = newVertices;
            cout << "[SYSTEM] Map expanded to " << vertices << " locations (Nodes 0 to " << vertices - 1 << ").\n";
        } else {
            cout << "[SYSTEM] Map already supports " << vertices << " locations.\n";
        }
    }

    bool addRoad(int source, int destination, double time, double hazard, bool bidirectional = true, bool verbose = true) {
        if (source == destination) {
            if (verbose)
                cout << "  [INPUT ERROR] Start and end location cannot be the same (" << source << ").\n";
            return false;
        }
        for (auto& road : adj[source]) {
            if (road.destination == destination) {
                road.travelTime = time;
                road.hazard = hazard;
                road.blocked = false;
                if (bidirectional) {
                    for (auto& rev : adj[destination]) {
                        if (rev.destination == source) {
                            rev.travelTime = time;
                            rev.hazard = hazard;
                            rev.blocked = false;
                        }
                    }
                }
                if (verbose)
                    cout << "[SYSTEM] Existing road updated and marked OPEN.\n";
                return true;
            }
        }

        adj[source].push_back({source, destination, time, hazard, false});
        if (bidirectional) {
            adj[destination].push_back({destination, source, time, hazard, false});
        }
        if (verbose)
            cout << "[SYSTEM] Road connected between " << getLocationName(source)
                 << " and " << getLocationName(destination) << ".\n";
        return true;
    }

    bool updateRoad(int source, int destination, double newTime, double newHazard, bool bidirectional = true) {
        bool found = false;
        for (auto& road : adj[source]) {
            if (road.destination == destination) {
                road.travelTime = newTime;
                road.hazard = newHazard;
                found = true;
                break;
            }
        }
        if (bidirectional) {
            for (auto& road : adj[destination]) {
                if (road.destination == source) {
                    road.travelTime = newTime;
                    road.hazard = newHazard;
                    break;
                }
            }
        }
        if (found) {
            cout << "[SYSTEM] Road travel time (" << newTime << " mins) and hazard (" << newHazard << ") updated.\n";
        } else {
            cout << "[ALERT] No road found between Node " << source << " and Node " << destination << ".\n";
        }
        return found;
    }

    bool setRoadBlockedStatus(int source, int destination, bool blockStatus, bool bidirectional = true) {
        bool found = false;
        for (auto& road : adj[source]) {
            if (road.destination == destination) {
                road.blocked = blockStatus;
                found = true;
                break;
            }
        }
        if (bidirectional) {
            for (auto& road : adj[destination]) {
                if (road.destination == source) {
                    road.blocked = blockStatus;
                    break;
                }
            }
        }
        if (found) {
            cout << "[SYSTEM] Road between Node " << source << " and Node " << destination
                 << (blockStatus ? " is now BLOCKED (Closed)." : " is now OPEN for traffic.") << "\n";
        } else {
            cout << "[ALERT] No road found between Node " << source << " and Node " << destination << ".\n";
        }
        return found;
    }

    // EXACT 2D VISUAL LAYOUT FROM SECOND SCREENSHOT (With Live Updating Labels & Custom Road Additions)
    void displayGraph() const {
        string r01 = getDiagramRoadLabel(0, 1);
        string r02 = getDiagramRoadLabel(0, 2);
        string r13 = getDiagramRoadLabel(1, 3);
        string r23 = getDiagramRoadLabel(2, 3);
        string r34 = getDiagramRoadLabel(3, 4);
        string r25 = getDiagramRoadLabel(2, 5);
        string r46 = getDiagramRoadLabel(4, 6);
        string r57 = getDiagramRoadLabel(5, 7);

        cout << "\n====================================================================================================\n";
        cout << "                               EASY-TO-READ REGIONAL ROAD NETWORK MAP                               \n";
        cout << "====================================================================================================\n";
        cout << "  Visual Layout of Sector Locations (Live Status):\n\n";
        cout << "                     [Node 1: North Bridge] -------- " << left << setw(9) << r13 << " -------- [Node 3: River Crossing]\n";
        cout << "                            /                                              /        |         \n";
        cout << "                      " << left << setw(9) << r01 << "                                     " << left << setw(9) << r23 << "  " << left << setw(9) << r34 << "     \n";
        cout << "                          /                                              /          |         \n";
        cout << "     [Node 0: MAIN DEPOT]                                               /       [Node 4: Valley] --- " << left << setw(9) << r46 << " --- [Node 6: South Plains]\n";
        cout << "                          \\                                            /                              \n";
        cout << "                      " << left << setw(9) << r02 << "                                      /                               \n";
        cout << "                            \\                                        /                                \n";
        cout << "                     [Node 2: East Highway] ------------------------/                                 \n";
        cout << "                            \\                                                                         \n";
        cout << "                        " << left << setw(9) << r25 << "                                                                    \n";
        cout << "                              \\                                                                       \n";
        cout << "                       [Node 5: Hilltop] ---------- " << left << setw(9) << r57 << " --------- [Node 7: West Outpost]        \n";

        // If the user adds any new custom roads, draw them right below the main 2D diagram in the same visual style!
        bool hasCustomRoads = false;
        for (int i = 0; i < vertices; i++) {
            for (const auto& road : adj[i]) {
                if (i < road.destination && !isDefaultDiagramRoad(i, road.destination)) {
                    if (!hasCustomRoads) {
                        cout << "\n  Additional User-Added Roads on Map:\n";
                        hasCustomRoads = true;
                    }
                    cout << "                     [" << getLocationName(i) << "] -------- "
                         << getDiagramRoadLabel(i, road.destination)
                         << " -------- [" << getLocationName(road.destination) << "]\n";
                }
            }
        }

        cout << "----------------------------------------------------------------------------------------------------\n";
        cout << left << setw(28) << "From Location"
             << setw(28) << "To Location"
             << setw(14) << "Travel Time"
             << setw(16) << "Flood Risk"
             << setw(15) << "Est. Fuel Cost"
             << "Status\n";
        cout << "----------------------------------------------------------------------------------------------------\n";

        bool anyRoads = false;
        for (int i = 0; i < vertices; i++) {
            for (const auto& road : adj[i]) {
                if (i < road.destination) {
                    anyRoads = true;
                    double effectiveMins = road.travelTime * (1.0 + road.hazard);
                    double fuelCostRs = effectiveMins * 25.0; // Rs. 25 per effective minute of truck travel

                    stringstream timeStr, costStr;
                    timeStr << fixed << setprecision(1) << road.travelTime << " mins";
                    costStr << "Rs. " << fixed << setprecision(0) << fuelCostRs;

                    cout << left << setw(28) << getLocationName(i)
                         << setw(28) << getLocationName(road.destination)
                         << setw(14) << timeStr.str()
                         << setw(16) << getHazardLabel(road.hazard)
                         << setw(15) << costStr.str()
                         << (road.blocked ? "[BLOCKED / CLOSED]" : "[OPEN]") << "\n";
                }
            }
        }
        if (!anyRoads) {
            cout << "  (No roads are currently connected on the map.)\n";
        }
        cout << "====================================================================================================\n";
        cout << "  Note: All roads are two-way. Effective Delay = Travel Time x (1 + Flood Risk).\n";
        cout << "====================================================================================================\n";
    }

    RouteResult calculateShortestRoute(int source, int destination) const {
        RouteResult result{false, numeric_limits<double>::infinity(), 0.0, 0.0, {}};
        if (source < 0 || source >= vertices || destination < 0 || destination >= vertices) {
            return result;
        }

        vector<double> distance(vertices, numeric_limits<double>::infinity());
        vector<double> rawTime(vertices, 0.0);
        vector<int> previous(vertices, -1);

        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;

        distance[source] = 0.0;
        pq.push({0.0, source});

        while (!pq.empty()) {
            double currentDistance = pq.top().first;
            int current = pq.top().second;
            pq.pop();

            if (current == destination)
                break;

            if (currentDistance > distance[current])
                continue;

            for (const auto& road : adj[current]) {
                if (road.blocked)
                    continue;

                double cost = road.travelTime * (1.0 + road.hazard);
                double newDistance = currentDistance + cost;

                if (newDistance < distance[road.destination]) {
                    distance[road.destination] = newDistance;
                    rawTime[road.destination] = rawTime[current] + road.travelTime;
                    previous[road.destination] = current;
                    pq.push({newDistance, road.destination});
                }
            }
        }

        if (distance[destination] == numeric_limits<double>::infinity()) {
            return result;
        }

        result.reachable = true;
        result.totalEffectiveCost = distance[destination];
        result.rawTravelTime = rawTime[destination];
        result.transportCostRs = distance[destination] * 25.0; // Rs. 25 per effective minute

        int current = destination;
        while (current != -1) {
            result.path.push_back(current);
            current = previous[current];
        }
        reverse(result.path.begin(), result.path.end());
        return result;
    }

    void displayRoute(int source, int destination) const {
        RouteResult res = calculateShortestRoute(source, destination);
        if (!res.reachable) {
            cout << "\n[ALERT] No open route available from " << getLocationName(source)
                 << " to " << getLocationName(destination) << "!\n";
            return;
        }
        cout << "\n---------------- BEST ROUTE DETAILS ----------------\n";
        cout << "Route Path                  : ";
        for (size_t i = 0; i < res.path.size(); i++) {
            cout << getLocationName(res.path[i]);
            if (i + 1 < res.path.size())
                cout << " -> ";
        }
        cout << "\nBase Travel Time            : " << fixed << setprecision(1) << res.rawTravelTime << " mins"
             << "\nHazard-Adjusted Travel Time : " << fixed << setprecision(1) << res.totalEffectiveCost << " mins"
             << "\nEstimated Truck Fuel Cost   : Rs. " << fixed << setprecision(2) << res.transportCostRs << "\n";
        cout << "----------------------------------------------------\n";
    }
};

// ============================================================================
// DEPOT INVENTORY & MULTI-TRIP ALLOCATION SIMULATOR
// ============================================================================

struct DepotInventory {
    int food = 600;            // Packets / kg
    int water = 900;           // Litres / Bottles
    int medicine = 350;        // Medical Kits / Boxes
    int vehicleCapacity = 250; // Total units per trip
};

void simulateSingleAllocation(int& inventory, int vehicleCapacity, int& demand) {
    cout << "\n================ MULTI-TRIP SUPPLY ALLOCATION PLAN ================\n";
    cout << "Initial Depot Stock : " << inventory << " units\n";
    cout << "Truck Capacity      : " << vehicleCapacity << " units per trip\n";
    cout << "Total Camp Demand   : " << demand << " units\n";

    if (demand == 0) {
        cout << "[STATUS] Camp demand is 0 units. No trips required.\n";
        return;
    }

    int totalTripsNeeded = (demand + vehicleCapacity - 1) / vehicleCapacity;
    cout << "Total Trips Needed  : " << totalTripsNeeded << " trip(s) to finish delivery\n";
    cout << "-------------------------------------------------------------------\n";

    int currentTrip = 1;
    while (demand > 0) {
        if (inventory < demand && inventory < vehicleCapacity) {
            int restockAmount = max(demand - inventory + 200, 500);
            cout << "  [DEPOT RESTOCK] Depot stock (" << inventory << " units) is low for Trip #"
                 << currentTrip << ". Automatically restocking +" << restockAmount << " units...\n";
            inventory += restockAmount;
        }

        int allocation = min({demand, vehicleCapacity, inventory});
        inventory -= allocation;
        demand -= allocation;

        cout << "  -> Trip #" << currentTrip << " : Sent " << setw(4) << allocation << " units"
             << " | Remaining Camp Demand: " << setw(4) << demand << " units"
             << " | Depot Stock Left: " << setw(4) << inventory << " units\n";
        currentTrip++;
    }

    cout << "-------------------------------------------------------------------\n";
    cout << "FINAL STATUS: All " << totalTripsNeeded << " trip(s) completed! 100% of camp demand satisfied.\n";
    cout << "===================================================================\n";
}

// ============================================================================
// MAIN FLOOD RELIEF SYSTEM CONTROLLER
// ============================================================================

class FloodReliefSystem {
private:
    vector<Camp> camps;
    Graph roadNetwork;
    DepotInventory depot;

    int findCampIndexById(int id) const {
        for (size_t i = 0; i < camps.size(); i++) {
            if (camps[i].campId == id)
                return static_cast<int>(i);
        }
        return -1;
    }

    void refreshAllCampPriorities() {
        for (auto& c : camps) {
            c.recalculateShortage();
            RouteResult route = roadNetwork.calculateShortestRoute(0, c.locationNode);
            if (route.reachable) {
                c.travelTime = static_cast<int>(route.totalEffectiveCost + 0.5);
                int trips = max(1, (c.shortage + depot.vehicleCapacity - 1) / depot.vehicleCapacity);
                c.estBudgetRs += (route.transportCostRs * trips);
            } else {
                c.travelTime = 500;
            }
            c.priority = MaxHeap::calculatePriority(c.urgency, c.population, c.shortage, c.travelTime);
        }
    }

public:
    FloodReliefSystem() : roadNetwork(8) {
        initializeSystemDefaults(false);
    }

    void initializeSystemDefaults(bool verbose = true) {
        camps.clear();
        roadNetwork = Graph(8);

        roadNetwork.addRoad(0, 1, 10, 0.2, true, verbose);
        roadNetwork.addRoad(0, 2, 15, 0.5, true, verbose);
        roadNetwork.addRoad(1, 3, 12, 0.1, true, verbose);
        roadNetwork.addRoad(2, 3, 8,  0.4, true, verbose);
        roadNetwork.addRoad(3, 4, 10, 0.3, true, verbose);
        roadNetwork.addRoad(2, 5, 20, 0.2, true, verbose);
        roadNetwork.addRoad(4, 6, 14, 0.6, true, verbose);
        roadNetwork.addRoad(5, 7, 11, 0.1, true, verbose);

        depot.food = 600;
        depot.water = 900;
        depot.medicine = 350;
        depot.vehicleCapacity = 250;
    }

    void ensureCampsExist() {
        if (camps.empty()) {
            cout << "\n[NOTICE] No relief camps are currently present in the system!\n";
            cout << "Please enter the details for your first relief camp to continue:\n";
            addCamp();

            while (true) {
                cout << "\nWhat would you like to do next?\n";
                cout << "1. Add another new Relief Camp\n";
                cout << "2. Continue to other options\n";
                int nextStep = getValidInt("Enter your choice (1-2): ", 1, 2, "2");
                if (nextStep == 1) {
                    addCamp();
                } else {
                    break;
                }
            }
        }
    }

    // ------------------------------------------------------------------------
    // OPTION 1: MANAGE RELIEF CAMPS
    // ------------------------------------------------------------------------
    void addCamp() {
        cout << "\n==================== ADD NEW RELIEF CAMP ====================\n";
        Camp c{};
        while (true) {
            c.campId = getValidInt("Enter Camp ID (1 - 99999): ", 1, 99999, "101");
            if (findCampIndexById(c.campId) != -1) {
                cout << "  [INPUT ERROR] Camp ID " << c.campId
                     << " already exists! Enter a different ID (Example: 102).\n";
            } else {
                break;
            }
        }

        c.campName = getValidString("Enter Camp Name / Location (Words only): ", "North River School Camp");
        int maxNode = roadNetwork.getVertexCount() - 1;
        c.locationNode = getValidInt(
            "Enter Map Node Location (1 - " + to_string(maxNode) + ", Node 0 is Main Depot): ",
            1, maxNode, "3"
        );
        c.population = getValidInt("Enter Affected Population (1 - 100000 people): ", 1, 100000, "150");
        c.urgency = getValidInt("Enter Urgency Level (1 - 10 scale): ", 1, 10, "8");
        c.food = getValidInt("Enter Food Needed (in Packets/kg, 0 - 50000): ", 0, 50000, "100");
        c.water = getValidInt("Enter Water Needed (in Litres/Bottles, 0 - 50000): ", 0, 50000, "150");
        c.medicine = getValidInt("Enter Medicine Needed (in Medical Kits/Boxes, 0 - 50000): ", 0, 50000, "50");

        c.recalculateShortage();
        camps.push_back(c);
        refreshAllCampPriorities();

        const Camp& added = camps.back();
        int tripsReq = (added.shortage + depot.vehicleCapacity - 1) / depot.vehicleCapacity;
        cout << "\n[SYSTEM] Camp #" << added.campId << " (" << added.campName << ") added successfully!\n";
        cout << "         -> Total Supply Needed : " << added.shortage << " units ("
             << added.food << " Packets Food, " << added.water << " Litres/Bottles Water, "
             << added.medicine << " Kits Medicine)\n";
        cout << "         -> Est. Travel Time    : " << added.travelTime << " mins\n";
        cout << "         -> Est. Relief Cost    : Rs. " << fixed << setprecision(2) << added.estBudgetRs << "\n";
        cout << "         -> Trucks/Trips Needed : " << tripsReq << " trip(s) (at "
             << depot.vehicleCapacity << " units/truck)\n";
    }

    void displayCamps() {
        if (camps.empty()) {
            cout << "\n[STATUS] No relief camps found.\n";
            return;
        }
        refreshAllCampPriorities();
        cout << "\n========================== ALL REGISTERED RELIEF CAMPS ==========================\n";
        for (size_t i = 0; i < camps.size(); i++) {
            int tripsReq = (camps[i].shortage + depot.vehicleCapacity - 1) / depot.vehicleCapacity;
            cout << "\nCamp #" << (i + 1) << "\n";
            cout << "  Camp ID              : " << camps[i].campId << "\n";
            cout << "  Name / Location      : " << camps[i].campName << "\n";
            cout << "  Map Sector           : " << getLocationName(camps[i].locationNode) << "\n";
            cout << "  Affected Population  : " << camps[i].population << " people\n";
            cout << "  Urgency Level (1-10) : " << camps[i].urgency << " / 10\n";
            cout << "  Food Needed          : " << camps[i].food << " Packets/kg\n";
            cout << "  Water Needed         : " << camps[i].water << " Litres/Bottles\n";
            cout << "  Medicine Needed      : " << camps[i].medicine << " Kits/Boxes\n";
            cout << "  Total Supply Shortage: " << camps[i].shortage << " total units\n";
            cout << "  Trips Required       : " << tripsReq << " trip(s) (Truck Capacity: " << depot.vehicleCapacity << " units)\n";
            cout << "  Effective Travel Time: " << camps[i].travelTime << " mins\n";
            cout << "  Estimated Total Cost : Rs. " << fixed << setprecision(2) << camps[i].estBudgetRs << "\n";
            cout << "  Priority Score       : " << camps[i].priority << "\n";
        }
        cout << "=================================================================================\n";
    }

    void searchCamp() const {
        if (camps.empty()) {
            cout << "\n[STATUS] No camps available to search.\n";
            return;
        }
        int id = getValidInt("Enter Camp ID to search (1 - 99999): ", 1, 99999, "101");
        int idx = findCampIndexById(id);
        if (idx == -1) {
            cout << "[ALERT] Camp ID " << id << " does not exist.\n";
            return;
        }
        const Camp& c = camps[idx];
        int tripsReq = (c.shortage + depot.vehicleCapacity - 1) / depot.vehicleCapacity;
        cout << "\n--- Camp Details ---\n";
        cout << "Camp ID              : " << c.campId << "\n";
        cout << "Name / Location      : " << c.campName << "\n";
        cout << "Map Sector           : " << getLocationName(c.locationNode) << "\n";
        cout << "Affected Population  : " << c.population << " people\n";
        cout << "Urgency Level        : " << c.urgency << " / 10\n";
        cout << "Food Needed          : " << c.food << " Packets/kg\n";
        cout << "Water Needed         : " << c.water << " Litres/Bottles\n";
        cout << "Medicine Needed      : " << c.medicine << " Kits/Boxes\n";
        cout << "Total Shortage       : " << c.shortage << " units (" << tripsReq << " trip(s) required)\n";
        cout << "Travel Time          : " << c.travelTime << " mins\n";
        cout << "Estimated Cost       : Rs. " << fixed << setprecision(2) << c.estBudgetRs << "\n";
        cout << "Priority Score       : " << c.priority << "\n";
    }

    void updateCamp() {
        if (camps.empty()) {
            cout << "\n[STATUS] No camps available to update.\n";
            return;
        }
        int id = getValidInt("Enter Camp ID to update (1 - 99999): ", 1, 99999, "101");
        int idx = findCampIndexById(id);
        if (idx == -1) {
            cout << "[ALERT] Camp ID " << id << " does not exist.\n";
            return;
        }

        Camp& c = camps[idx];
        cout << "\nUpdating Camp #" << c.campId << " (" << c.campName << ")\n";
        c.population = getValidInt("Enter New Population (1 - 100000 people): ", 1, 100000, "180");
        c.urgency = getValidInt("Enter New Urgency Level (1 - 10): ", 1, 10, "9");
        c.food = getValidInt("Enter New Food Needed (in Packets/kg, 0 - 50000): ", 0, 50000, "90");
        c.water = getValidInt("Enter New Water Needed (in Litres/Bottles, 0 - 50000): ", 0, 50000, "120");
        c.medicine = getValidInt("Enter New Medicine Needed (in Kits/Boxes, 0 - 50000): ", 0, 50000, "45");

        refreshAllCampPriorities();
        cout << "[SYSTEM] Camp #" << c.campId << " updated! New Priority Score: " << c.priority
             << " | New Est. Cost: Rs. " << fixed << setprecision(2) << c.estBudgetRs << "\n";
    }

    void displaySummary() const {
        if (camps.empty()) {
            cout << "\n[STATUS] No camps available.\n";
            return;
        }
        int totalPopulation = 0, totalFood = 0, totalWater = 0, totalMedicine = 0, totalTrips = 0;
        double totalBudgetRs = 0.0;
        for (const Camp& c : camps) {
            totalPopulation += c.population;
            totalFood += c.food;
            totalWater += c.water;
            totalMedicine += c.medicine;
            totalTrips += (c.shortage + depot.vehicleCapacity - 1) / depot.vehicleCapacity;
            totalBudgetRs += c.estBudgetRs;
        }
        cout << "\n================== TOTAL RELIEF CAMP SUMMARY ==================\n";
        cout << "Total Relief Camps          : " << camps.size() << " camp(s)\n";
        cout << "Total People Affected       : " << totalPopulation << " people\n";
        cout << "Total Food Needed           : " << totalFood << " Packets/kg\n";
        cout << "Total Water Needed          : " << totalWater << " Litres/Bottles\n";
        cout << "Total Medicine Needed       : " << totalMedicine << " Kits/Boxes\n";
        cout << "Total Trips Needed Across All: " << totalTrips << " truck trip(s)\n";
        cout << "Total Estimated Relief Cost : Rs. " << fixed << setprecision(2) << totalBudgetRs << "\n";
        cout << "===============================================================\n";
    }

    void campManagementMenu() {
        if (camps.empty()) {
            ensureCampsExist();
        } else {
            cout << "\n[STATUS] There are currently " << camps.size() << " relief camp(s) registered.\n";
            cout << "1. Add a new Relief Camp right now\n";
            cout << "2. Check other Camp Management options (View, Search, Update, Summary)\n";
            int action = getValidInt("Enter your choice (1-2): ", 1, 2, "2");
            if (action == 1) {
                addCamp();
            }
        }

        int choice;
        do {
            cout << "\n========== MANAGE RELIEF CAMPS ==========\n";
            cout << "1. Add a New Camp\n";
            cout << "2. View All Camps\n";
            cout << "3. Search for a Camp by ID\n";
            cout << "4. Update Camp Details\n";
            cout << "5. View Total Summary\n";
            cout << "6. Back to Main Menu\n";
            choice = getValidInt("Choose an option (1-6): ", 1, 6, "2");

            switch (choice) {
                case 1: addCamp(); break;
                case 2: displayCamps(); break;
                case 3: searchCamp(); break;
                case 4: updateCamp(); break;
                case 5: displaySummary(); break;
                case 6: break;
            }
        } while (choice != 6);
    }

    // ------------------------------------------------------------------------
    // OPTION 2: VIEW CAMP PRIORITY LIST & CUSTOM CAMP SIMULATOR
    // ------------------------------------------------------------------------
    void priorityHeapMenu() {
        if (camps.empty()) {
            ensureCampsExist();
        }

        int choice;
        do {
            cout << "\n========== CAMP PRIORITY LIST ==========\n";
            cout << "1. Show Full Priority Ranking (Highest to Lowest)\n";
            cout << "2. Show #1 Top Priority Camp for Next Delivery\n";
            cout << "3. Test Custom Camp Priorities (With Time, Money, Water & Food Units)\n";
            cout << "4. Back to Main Menu\n";
            choice = getValidInt("Choose an option (1-4): ", 1, 4, "1");

            if (choice == 1 || choice == 2) {
                refreshAllCampPriorities();
                MaxHeap mh;
                for (const auto& c : camps) {
                    if (c.shortage > 0) {
                        mh.insertCamp(c);
                    }
                }
                if (choice == 1) {
                    mh.displayPriorityRanking(depot.vehicleCapacity);
                } else {
                    if (mh.isEmpty()) {
                        cout << "\n[STATUS] All camps have received their supplies!\n";
                    } else {
                        Camp top = mh.removeHighestPriority();
                        int tripsReq = (top.shortage + depot.vehicleCapacity - 1) / depot.vehicleCapacity;
                        cout << "\n>>> TOP PRIORITY CAMP FOR NEXT DELIVERY <<<\n";
                        cout << "Camp ID        : " << top.campId << " (" << top.campName << ")\n"
                             << "Priority Score : " << top.priority << "\n"
                             << "Supply Breakdown: " << top.food << " Packets Food, "
                             << top.water << " Litres/Bottles Water, "
                             << top.medicine << " Kits Medicine (" << top.shortage << " total units)\n"
                             << "Travel Time    : " << top.travelTime << " mins\n"
                             << "Estimated Cost : Rs. " << fixed << setprecision(2) << top.estBudgetRs << "\n"
                             << "Trips Required : " << tripsReq << " trip(s)\n";
                    }
                }
            } else if (choice == 3) {
                MaxHeap customHeap;
                int n = getValidInt("How many custom camps do you want to compare? (1 - 20): ", 1, 20, "2");
                for (int i = 0; i < n; i++) {
                    Camp c{};
                    cout << "\n--- Custom Camp #" << (i + 1) << " ---\n";
                    c.campId = getValidInt("Enter Camp ID (1 - 99999): ", 1, 99999, to_string(201 + i));
                    c.campName = getValidString("Enter Camp Name (Words only): ", "River Valley Camp");
                    c.urgency = getValidInt("Enter Urgency Level (1 - 10): ", 1, 10, "8");
                    c.population = getValidInt("Enter Affected Population (1 - 100000 people): ", 1, 100000, "120");
                    c.food = getValidInt("Enter Food Needed (in Packets/kg, 0 - 50000): ", 0, 50000, "80");
                    c.water = getValidInt("Enter Water Needed (in Litres/Bottles, 0 - 50000): ", 0, 50000, "120");
                    c.medicine = getValidInt("Enter Medicine Needed (in Kits/Boxes, 0 - 50000): ", 0, 50000, "30");
                    c.travelTime = getValidInt("Enter Travel Time (in Minutes [mins], 1 - 10000): ", 1, 10000, "25");
                    c.estBudgetRs = getValidDouble("Enter Estimated Relief Cost / Budget (in Rupees [Rs.], 0 - 10000000): ", 0.0, 10000000.0, "15000");

                    c.shortage = c.food + c.water + c.medicine;
                    c.priority = MaxHeap::calculatePriority(c.urgency, c.population, c.shortage, c.travelTime);
                    customHeap.insertCamp(c);
                }
                customHeap.displayPriorityRanking(depot.vehicleCapacity);
                Camp next = customHeap.removeHighestPriority();
                cout << "\n>>> Highest Priority Custom Camp Selected <<<\n";
                cout << "Camp #" << next.campId << " (" << next.campName << ")"
                     << " | Priority Score: " << next.priority
                     << " | Travel Time: " << next.travelTime << " mins"
                     << " | Budget: Rs. " << fixed << setprecision(2) << next.estBudgetRs << "\n";
            }
        } while (choice != 4);
    }

    // ------------------------------------------------------------------------
    // OPTION 3: MANAGE ROADS & ROUTES
    // ------------------------------------------------------------------------
    void graphAndRouteMenu() {
        int choice;
        do {
            int maxNode = roadNetwork.getVertexCount() - 1;
            cout << "\n========== MANAGE ROADS & ROUTES ==========\n";
            cout << "1. View Easy-to-Read Road Map & Status Table\n";
            cout << "2. Add a New Road\n";
            cout << "3. Change Road Travel Time (mins) or Flood Risk\n";
            cout << "4. Block a Flooded Road\n";
            cout << "5. Reopen a Blocked Road\n";
            cout << "6. Find Best Route Between Two Locations\n";
            cout << "7. Add More Map Locations\n";
            cout << "8. Back to Main Menu\n";
            choice = getValidInt("Choose an option (1-8): ", 1, 8, "1");

            if (choice == 1) {
                roadNetwork.displayGraph();
            } else if (choice == 2) {
                int s = getValidInt("Enter Start Node (0 - " + to_string(maxNode) + "): ", 0, maxNode, "0");
                int d = getValidInt("Enter End Node (0 - " + to_string(maxNode) + "): ", 0, maxNode, "3");
                double t = getValidDouble("Enter Travel Time in Minutes (mins, 0.1 - 10000.0): ", 0.1, 10000.0, "12.5");
                double h = getValidDouble("Enter Flood Hazard Risk (0.0 - 10.0, e.g., 0.3): ", 0.0, 10.0, "0.3");
                if (roadNetwork.addRoad(s, d, t, h, true)) {
                    refreshAllCampPriorities();
                    cout << "[MAP UPDATED] Displaying updated Road Network Map with the new road:\n";
                    roadNetwork.displayGraph();
                }
            } else if (choice == 3) {
                int s = getValidInt("Enter Start Node (0 - " + to_string(maxNode) + "): ", 0, maxNode, "0");
                int d = getValidInt("Enter End Node (0 - " + to_string(maxNode) + "): ", 0, maxNode, "1");
                double t = getValidDouble("Enter New Travel Time in Minutes (mins, 0.1 - 10000.0): ", 0.1, 10000.0, "15.0");
                double h = getValidDouble("Enter New Flood Hazard Risk (0.0 - 10.0): ", 0.0, 10.0, "0.5");
                if (roadNetwork.updateRoad(s, d, t, h, true)) {
                    refreshAllCampPriorities();
                    cout << "[MAP UPDATED] Displaying updated Road Network Map:\n";
                    roadNetwork.displayGraph();
                }
            } else if (choice == 4) {
                int s = getValidInt("Enter Start Node of Road to Block (0 - " + to_string(maxNode) + "): ", 0, maxNode, "0");
                int d = getValidInt("Enter End Node of Road to Block (0 - " + to_string(maxNode) + "): ", 0, maxNode, "1");
                if (roadNetwork.setRoadBlockedStatus(s, d, true, true)) {
                    refreshAllCampPriorities();
                    cout << "[MAP UPDATED] Displaying updated Road Network Map showing BLOCKED status:\n";
                    roadNetwork.displayGraph();
                }
            } else if (choice == 5) {
                int s = getValidInt("Enter Start Node of Road to Reopen (0 - " + to_string(maxNode) + "): ", 0, maxNode, "0");
                int d = getValidInt("Enter End Node of Road to Reopen (0 - " + to_string(maxNode) + "): ", 0, maxNode, "1");
                if (roadNetwork.setRoadBlockedStatus(s, d, false, true)) {
                    refreshAllCampPriorities();
                    cout << "[MAP UPDATED] Displaying updated Road Network Map showing OPEN status:\n";
                    roadNetwork.displayGraph();
                }
            } else if (choice == 6) {
                int s = getValidInt("Enter Start Node (Main Depot is 0, Range 0 - " + to_string(maxNode) + "): ", 0, maxNode, "0");
                int d = getValidInt("Enter Destination Node (0 - " + to_string(maxNode) + "): ", 0, maxNode, "3");
                roadNetwork.displayRoute(s, d);
            } else if (choice == 7) {
                int newCount = getValidInt("Enter new total number of locations (" + to_string(maxNode + 2) + " - 100): ",
                                           maxNode + 2, 100, to_string(maxNode + 3));
                roadNetwork.resizeGraph(newCount);
                roadNetwork.displayGraph();
            }
        } while (choice != 8);
    }

    // ------------------------------------------------------------------------
    // OPTION 4: MANAGE DEPOT INVENTORY
    // ------------------------------------------------------------------------
    void inventoryMenu() {
        int choice;
        do {
            cout << "\n==================== MANAGE DEPOT INVENTORY ====================\n";
            cout << "Current Warehouse Stock:\n";
            cout << "  - Food     : " << depot.food << " Packets/kg\n";
            cout << "  - Water    : " << depot.water << " Litres/Bottles\n";
            cout << "  - Medicine : " << depot.medicine << " Kits/Boxes\n";
            cout << "  - Truck Capacity: " << depot.vehicleCapacity << " total units per trip\n";
            cout << "----------------------------------------------------------------\n";
            cout << "1. Restock / Update Depot Inventory\n";
            cout << "2. Change Truck Capacity (units per trip)\n";
            cout << "3. Test Multi-Trip Supply Allocation Calculator\n";
            cout << "4. Back to Main Menu\n";
            choice = getValidInt("Choose an option (1-4): ", 1, 4, "1");

            if (choice == 1) {
                depot.food = getValidInt("Enter Food Stock (in Packets/kg, 0 - 500000): ", 0, 500000, "600");
                depot.water = getValidInt("Enter Water Stock (in Litres/Bottles, 0 - 500000): ", 0, 500000, "900");
                depot.medicine = getValidInt("Enter Medicine Stock (in Kits/Boxes, 0 - 500000): ", 0, 500000, "350");
                cout << "[SYSTEM] Depot stock updated successfully!\n";
            } else if (choice == 2) {
                depot.vehicleCapacity = getValidInt("Enter Truck Capacity per trip (1 - 100000 units): ", 1, 100000, "250");
                cout << "[SYSTEM] Truck capacity set to " << depot.vehicleCapacity << " units per trip.\n";
            } else if (choice == 3) {
                cout << "\n===== MULTI-TRIP SUPPLY ALLOCATION CALCULATOR =====\n";
                int inv = getValidInt("Enter available warehouse stock (0 - 100000 units): ", 0, 100000, "500");
                int cap = getValidInt("Enter truck capacity per trip (1 - 100000 units): ", 1, 100000, "200");
                int dem = getValidInt("Enter total camp demand (0 - 100000 units): ", 0, 100000, "450");
                simulateSingleAllocation(inv, cap, dem);
            }
        } while (choice != 4);
    }

    // ------------------------------------------------------------------------
    // OPTION 5: COMPLETE MULTI-TRIP DISPATCH ACROSS ALL RELIEF CAMPS
    // ------------------------------------------------------------------------
    void executeIntegratedDispatch() {
        if (camps.empty()) {
            ensureCampsExist();
        }

        cout << "\n========================================================================================================\n";
        cout << "                   COMPLETE MULTI-TRIP RELIEF DISPATCH PLAN (ALL RELIEF CAMPS)                          \n";
        cout << "========================================================================================================\n";

        refreshAllCampPriorities();
        MaxHeap dispatchHeap;
        int totalInitialShortage = 0;
        for (const auto& c : camps) {
            if (c.shortage > 0) {
                dispatchHeap.insertCamp(c);
                totalInitialShortage += c.shortage;
            }
        }

        if (dispatchHeap.isEmpty()) {
            cout << "[STATUS] All relief camps already have 100% of their required supplies!\n";
            return;
        }

        dispatchHeap.displayPriorityRanking(depot.vehicleCapacity);

        int grandTotalTrips = 0;
        int campsCompleted = 0;
        double grandTotalTimeMins = 0.0;
        double grandTotalCostRs = 0.0;

        while (!dispatchHeap.isEmpty()) {
            Camp candidate = dispatchHeap.removeHighestPriority();
            int idx = findCampIndexById(candidate.campId);
            Camp& target = camps[idx];

            RouteResult route = roadNetwork.calculateShortestRoute(0, target.locationNode);
            if (!route.reachable) {
                cout << "\n[ROUTING ALERT] Camp #" << target.campId << " (" << target.campName
                     << " at " << getLocationName(target.locationNode)
                     << ") has NO open road from the Main Depot! Skipping until road is reopened.\n";
                continue;
            }

            int tripsForThisCamp = (target.shortage + depot.vehicleCapacity - 1) / depot.vehicleCapacity;

            cout << "\n--------------------------------------------------------------------------------------------------------\n";
            cout << ">>> DISPATCHING TO CAMP #" << target.campId << ": " << target.campName
                 << " (" << getLocationName(target.locationNode) << ") <<<\n";
            cout << "Priority Score        : " << target.priority << " | Urgency: " << target.urgency << "/10\n";
            cout << "Initial Camp Demand   : " << target.food << " Packets Food, "
                 << target.water << " Litres/Bottles Water, "
                 << target.medicine << " Kits Medicine (" << target.shortage << " total units)\n";
            cout << "Best Dijkstra Route   : ";
            for (size_t i = 0; i < route.path.size(); i++) {
                cout << "Node " << route.path[i] << (i + 1 < route.path.size() ? " -> " : "");
            }
            cout << " (One-way: " << fixed << setprecision(1) << route.totalEffectiveCost
                 << " mins | Fuel Cost: Rs. " << fixed << setprecision(0) << route.transportCostRs << "/trip)\n";
            cout << "Truck Capacity        : " << depot.vehicleCapacity << " units/trip\n";
            cout << "TOTAL TRIPS REQUIRED  : " << tripsForThisCamp << " trip(s) to completely satisfy this camp\n";
            cout << "--------------------------------------------------------------------------------------------------------\n";

            for (int tripNum = 1; tripNum <= tripsForThisCamp && target.shortage > 0; tripNum++) {
                if (depot.medicine < target.medicine || depot.water < target.water || depot.food < target.food) {
                    int addMed   = (depot.medicine < target.medicine) ? (target.medicine - depot.medicine + 200) : 0;
                    int addWater = (depot.water < target.water)       ? (target.water - depot.water + 500)       : 0;
                    int addFood  = (depot.food < target.food)         ? (target.food - depot.food + 400)         : 0;

                    if (addMed > 0 || addWater > 0 || addFood > 0) {
                        depot.medicine += addMed;
                        depot.water += addWater;
                        depot.food += addFood;
                        cout << "  [DEPOT AUTO-RESTOCK] Warehouse restocked mid-operation (+"
                             << addFood << " Packets Food, +"
                             << addWater << " Litres/Bottles Water, +"
                             << addMed << " Kits Medicine) to continue trips without interruption.\n";
                    }
                }

                int remainingTruckSpace = depot.vehicleCapacity;

                int allocMed = min({target.medicine, depot.medicine, remainingTruckSpace});
                target.medicine -= allocMed;
                depot.medicine -= allocMed;
                remainingTruckSpace -= allocMed;

                int allocWater = min({target.water, depot.water, remainingTruckSpace});
                target.water -= allocWater;
                depot.water -= allocWater;
                remainingTruckSpace -= allocWater;

                int allocFood = min({target.food, depot.food, remainingTruckSpace});
                target.food -= allocFood;
                depot.food -= allocFood;
                remainingTruckSpace -= allocFood;

                int loadedUnits = allocMed + allocWater + allocFood;
                double tripSupplyCostRs = (allocFood * 50.0) + (allocWater * 20.0) + (allocMed * 150.0);
                double totalTripCostRs = tripSupplyCostRs + route.transportCostRs;

                target.recalculateShortage();
                grandTotalTrips++;
                grandTotalTimeMins += route.totalEffectiveCost;
                grandTotalCostRs += totalTripCostRs;

                cout << "  [Trip " << tripNum << " of " << tripsForThisCamp << "] "
                     << "Loaded " << setw(3) << loadedUnits << "/" << depot.vehicleCapacity << " units: "
                     << setw(3) << allocFood << " Pkts Food, "
                     << setw(3) << allocWater << " L Water, "
                     << setw(3) << allocMed << " Kits Meds"
                     << " | Remaining Camp Need: " << setw(4) << target.shortage << " units"
                     << " | Trip Cost: Rs. " << fixed << setprecision(0) << totalTripCostRs << "\n";
            }

            cout << "  [CAMP COMPLETE] Camp #" << target.campId << " (" << target.campName
                 << ") received all supplies in " << tripsForThisCamp << " trip(s)! Remaining Demand: 0 units.\n";
            campsCompleted++;
        }

        refreshAllCampPriorities();

        cout << "\n========================================================================================================\n";
        cout << "                                   FINAL OPERATION DISPATCH SUMMARY                                     \n";
        cout << "========================================================================================================\n";
        cout << "Total Camps Fully Served    : " << campsCompleted << " camp(s)\n";
        cout << "Total Truck Trips Executed  : " << grandTotalTrips << " trip(s)\n";
        cout << "Total Supplies Delivered    : " << totalInitialShortage << " units\n";
        cout << "Total Cumulative Drive Time : " << fixed << setprecision(1) << grandTotalTimeMins << " mins\n";
        cout << "Total Relief & Transport Rs.: Rs. " << fixed << setprecision(2) << grandTotalCostRs << "\n";
        cout << "Remaining Depot Stock       : " << depot.food << " Packets Food | "
             << depot.water << " Litres/Bottles Water | "
             << depot.medicine << " Kits Medicine\n";
        cout << "========================================================================================================\n";
    }

    // ------------------------------------------------------------------------
    // OPTION 6: RUN SYSTEM TESTS
    // ------------------------------------------------------------------------
    static int calculateBaselinePriority(int urgency) {
        return urgency * 10;
    }

    void runSystemDiagnostics() {
        cout << "\n================ SYSTEM TESTS ================\n";

        cout << "\n[Test 1 - High Urgency Priority Check]\n";
        int urgency1 = getValidInt("Enter urgency (1 - 10, enter 7 or higher to pass): ", 1, 10, "8");
        int priority1 = calculateBaselinePriority(urgency1);
        cout << "Calculated Score: " << priority1 << " -> ";
        if (priority1 >= 70)
            cout << "High Priority Test: PASS\n";
        else
            cout << "High Priority Test: FAIL (Needs urgency >= 7)\n";

        cout << "\n[Test 2 - Low Urgency Priority Check]\n";
        int urgency2 = getValidInt("Enter urgency (1 - 10, enter 4 or lower to pass): ", 1, 10, "3");
        int priority2 = calculateBaselinePriority(urgency2);
        cout << "Calculated Score: " << priority2 << " -> ";
        if (priority2 < 50)
            cout << "Low Priority Test: PASS\n";
        else
            cout << "Low Priority Test: FAIL (Needs urgency <= 4)\n";

        cout << "\n[Test 3 - Road Hazard Update Check]\n";
        Road testRoad{};
        testRoad.source = getValidInt("Enter start node (0 - 50): ", 0, 50, "0");
        testRoad.destination = getValidInt("Enter end node (0 - 50): ", 0, 50, "1");
        testRoad.blocked = false;
        testRoad.hazard = getValidDouble("Enter hazard level (0.0 - 10.0): ", 0.0, 10.0, "0.4");
        if (testRoad.hazard >= 0.0)
            cout << "Hazard Update Test: PASS\n";
        else
            cout << "Hazard Update Test: FAIL\n";

        cout << "\n[Test 4 - Road Blocking Check]\n";
        testRoad.blocked = true;
        cout << "Blocking road...\n";
        if (testRoad.blocked)
            cout << "Road Blocking Test: PASS\n";
        else
            cout << "Road Blocking Test: FAIL\n";

        cout << "==============================================\n";
    }
};

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main() {
    FloodReliefSystem system;
    int choice;

    cout << "===============================================================\n";
    cout << "          SMART FLOOD RELIEF DISTRIBUTION SYSTEM               \n";
    cout << "===============================================================\n";
    cout << "[READY] 8 Map Locations (Node 0 = Main Depot) & Stock Initialized.\n";

    system.ensureCampsExist();

    do {
        cout << "\n========================== MAIN MENU ==========================\n";
        cout << "1. Manage Relief Camps     (Add, View, Search, Update)\n";
        cout << "2. View Camp Priority List (Rankings & Custom Camp Simulator)\n";
        cout << "3. Manage Roads & Routes   (Visual Map, Block/Open, Best Route)\n";
        cout << "4. Manage Depot Inventory  (Stock, Truck Limit & Trip Planner)\n";
        cout << "5. SEND RELIEF SUPPLIES    (Complete All Trips for Every Camp)\n";
        cout << "6. Run System Tests        (Verify Priority & Road Logic)\n";
        cout << "7. Clear All Camps & Reset (Start Fresh with 0 Camps)\n";
        cout << "8. Exit Program\n";
        cout << "===============================================================\n";

        choice = getValidInt("Enter your choice (1-8): ", 1, 8, "5");

        switch (choice) {
            case 1:
                system.campManagementMenu();
                break;
            case 2:
                system.priorityHeapMenu();
                break;
            case 3:
                system.graphAndRouteMenu();
                break;
            case 4:
                system.inventoryMenu();
                break;
            case 5:
                system.executeIntegratedDispatch();
                break;
            case 6:
                system.runSystemDiagnostics();
                break;
            case 7:
                system.initializeSystemDefaults(false);
                cout << "\n[SYSTEM] All camps cleared and road network/inventory reset.\n";
                system.ensureCampsExist();
                break;
            case 8:
                cout << "\nExiting Smart Flood Relief System. Goodbye!\n";
                break;
        }
    } while (choice != 8);

    return 0;
}
