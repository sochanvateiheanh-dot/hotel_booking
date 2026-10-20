#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;

// Simple Date Structure for parsing and range calculations
struct Date
{
    int year, month, day;

    // Converts date to an approximate linear day integer (sufficient for duration & overlap checks)
    long toDays() const
    {
        // Standard formula converting YMD to cumulative day count
        int y = year;
        int m = month;
        if (m <= 2)
        {
            y -= 1;
            m += 12;
        }
        return 365L * y + y / 4 - y / 100 + y / 400 + (153 * (m - 3) + 2) / 5 + day;
    }

    bool operator<(const Date &other) const
    {
        return toDays() < other.toDays();
    }

    bool operator>(const Date &other) const
    {
        return toDays() > other.toDays();
    }

    string toString() const
    {
        stringstream ss;
        ss << year << "-"
           << (month < 10 ? "0" : "") << month << "-"
           << (day < 10 ? "0" : "") << day;
        return ss.str();
    }
};

// Represents an individual reservation record
struct ReservationRecord
{
    string customerName;
    int roomNumber;
    Date checkIn;
    Date checkOut;
};

// Represents an individual hotel room
class Room
{
public:
    int roomNumber;
    string roomType; // Single, Double, Deluxe, Twin, Family, Suite
    string bedSetup; // 1 Single, 1 Double, 2 Twin, 1 King, 2 Queen
    int maxGuests;   // Max occupancy limit
    double pricePerNight;

    Room() : roomNumber(0), roomType(""), bedSetup(""), maxGuests(0), pricePerNight(0.0) {}

    Room(int num, string type, string bed, int maxG, double price)
    {
        roomNumber = num;
        roomType = type;
        bedSetup = bed;
        maxGuests = maxG;
        pricePerNight = price;
    }
};

// Clears cin failure flags and flushes remaining characters from stream
void clearInput()
{
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Parses a date from YYYY-MM-DD string
bool parseDate(const string &str, Date &d)
{
    if (str.length() != 10 || str[4] != '-' || str[7] != '-')
        return false;
    try
    {
        d.year = stoi(str.substr(0, 4));
        d.month = stoi(str.substr(5, 2));
        d.day = stoi(str.substr(8, 2));
        if (d.month < 1 || d.month > 12 || d.day < 1 || d.day > 31 || d.year < 2026)
        {
            return false;
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}

// Loads historical bookings into a vector for date-overlap checks
vector<ReservationRecord> loadAllReservations()
{
    vector<ReservationRecord> records;
    ifstream file("bookings.txt");
    if (!file.is_open())
        return records;

    string line;
    while (getline(file, line))
    {
        // Expected format: Room: [num] | In: [date] | Out: [date]
        size_t roomPos = line.find("Room: ");
        size_t inPos = line.find("| In: ");
        size_t outPos = line.find("| Out: ");

        if (roomPos != string::npos && inPos != string::npos && outPos != string::npos)
        {
            ReservationRecord rec;
            rec.roomNumber = stoi(line.substr(roomPos + 6, inPos - (roomPos + 6)));
            string inStr = line.substr(inPos + 6, 10);
            string outStr = line.substr(outPos + 7, 10);
            if (parseDate(inStr, rec.checkIn) && parseDate(outStr, rec.checkOut))
            {
                records.push_back(rec);
            }
        }
    }
    file.close();
    return records;
}

// Checks if a specific room is free during the requested interval [checkIn, checkOut)
bool isRoomAvailable(int roomNumber, const Date &checkIn, const Date &checkOut, const vector<ReservationRecord> &existing)
{
    for (const auto &rec : existing)
    {
        if (rec.roomNumber == roomNumber)
        {
            // Standard interval intersection check:
            // An overlap occurs if Requested_In < Existing_Out AND Requested_Out > Existing_In
            if (checkIn < rec.checkOut && checkOut > rec.checkIn)
            {
                return false;
            }
        }
    }
    return true;
}

// Displays all existing rooms in the inventory
void displayAllRooms(const Room rooms[], int totalRooms)
{
    cout << "\n================================ HOTEL INVENTORY ================================\n";
    cout << left << setw(8) << "Room #"
         << setw(16) << "Room Type"
         << setw(18) << "Bed Layout"
         << setw(12) << "Max Guests"
         << setw(14) << "Price/Night" << "\n";
    cout << string(68, '-') << "\n";

    for (int i = 0; i < totalRooms; ++i)
    {
        cout << left << setw(8) << rooms[i].roomNumber
             << setw(16) << rooms[i].roomType
             << setw(18) << rooms[i].bedSetup
             << setw(12) << rooms[i].maxGuests
             << "$" << fixed << setprecision(2) << rooms[i].pricePerNight << "\n";
    }
}

// Interactive Real-World Booking Flow
void bookRoomsFlow(Room rooms[], int totalRooms)
{
    vector<ReservationRecord> existingReservations = loadAllReservations();

    cout << "\n=================== REAL-LIFE HOTEL RESERVATION ===================\n";
    string guestName;
    cout << "Enter primary guest full name: ";
    getline(cin, guestName);
    if (guestName.empty())
    {
        cout << "Error: Guest name cannot be blank.\n";
        return;
    }

    // Step 1: Stay Dates
    Date checkIn, checkOut;
    string dateStr;

    cout << "Enter Check-in Date (YYYY-MM-DD): ";
    cin >> dateStr;
    if (!parseDate(dateStr, checkIn))
    {
        cout << "Error: Invalid date format. Must be YYYY-MM-DD.\n";
        clearInput();
        return;
    }

    cout << "Enter Check-out Date (YYYY-MM-DD): ";
    cin >> dateStr;
    if (!parseDate(dateStr, checkOut))
    {
        cout << "Error: Invalid date format. Must be YYYY-MM-DD.\n";
        clearInput();
        return;
    }

    long totalNights = checkOut.toDays() - checkIn.toDays();
    if (totalNights <= 0)
    {
        cout << "Error: Check-out date must be at least 1 day after check-in date.\n";
        clearInput();
        return;
    }

    // Step 2: Party Size (Adults, Children) and Rooms Needed
    int adults = 0, children = 0, roomsNeeded = 0;
    cout << "Enter number of adults: ";
    if (!(cin >> adults) || adults < 1)
    {
        cout << "Error: At least 1 adult is required.\n";
        clearInput();
        return;
    }

    cout << "Enter number of children: ";
    if (!(cin >> children) || children < 0)
    {
        cout << "Error: Invalid number of children.\n";
        clearInput();
        return;
    }

    cout << "Enter number of rooms required: ";
    if (!(cin >> roomsNeeded) || roomsNeeded < 1)
    {
        cout << "Error: You must book at least 1 room.\n";
        clearInput();
        return;
    }

    int totalGuests = adults + children;

    // Step 3: Room Type Selection
    cout << "\nSelect Desired Room Category:\n";
    cout << "1. Single-bed Room  (1 Single Bed | Max 1 guest)\n";
    cout << "2. Double Room      (1 Double Bed | Max 2 guests)\n";
    cout << "3. Twin Bed Room    (2 Twin Beds  | Max 2 guests)\n";
    cout << "4. Deluxe Room      (1 King Bed   | Max 3 guests)\n";
    cout << "5. Family Room      (2 Queen Beds | Max 4 guests)\n";
    cout << "6. Luxury Suite     (1 King + Sofa Bed | Max 5 guests)\n";
    cout << "Enter choice (1-6): ";

    int choice;
    if (!(cin >> choice) || choice < 1 || choice > 6)
    {
        cout << "Error: Invalid choice.\n";
        clearInput();
        return;
    }
    clearInput();

    string selectedType;
    switch (choice)
    {
    case 1:
        selectedType = "Single";
        break;
    case 2:
        selectedType = "Double";
        break;
    case 3:
        selectedType = "Twin";
        break;
    case 4:
        selectedType = "Deluxe";
        break;
    case 5:
        selectedType = "Family";
        break;
    case 6:
        selectedType = "Suite";
        break;
    }

    // Step 4: Search for rooms matching category, capacity, and date availability
    vector<int> candidateRoomIndices;
    int maxCapacitySum = 0;

    for (int i = 0; i < totalRooms; ++i)
    {
        if (rooms[i].roomType == selectedType)
        {
            if (isRoomAvailable(rooms[i].roomNumber, checkIn, checkOut, existingReservations))
            {
                candidateRoomIndices.push_back(i);
                maxCapacitySum += rooms[i].maxGuests;
            }
        }
    }

    // Check if enough rooms of this type exist for the chosen dates
    if (static_cast<int>(candidateRoomIndices.size()) < roomsNeeded)
    {
        cout << "\nUnavailable: Only " << candidateRoomIndices.size()
             << " " << selectedType << " room(s) available for the selected dates.\n";
        return;
    }

    // Check if the requested number of rooms can accommodate all guests
    int totalBookedCapacity = 0;
    for (int i = 0; i < roomsNeeded; ++i)
    {
        totalBookedCapacity += rooms[candidateRoomIndices[i]].maxGuests;
    }

    if (totalGuests > totalBookedCapacity)
    {
        cout << "\nCapacity Error: " << roomsNeeded << " " << selectedType
             << " room(s) can host a maximum of " << totalBookedCapacity
             << " guests, but your group has " << totalGuests << " guests.\n"
             << "Please book additional rooms or select a larger category (e.g., Family or Suite).\n";
        return;
    }

    // Step 5: Assign rooms, calculate costs, and save to file
    ofstream file("bookings.txt", ios::app);
    if (!file.is_open())
    {
        cout << "Error: Could not open bookings.txt to save records.\n";
        return;
    }

    double grandTotal = 0.0;
    vector<int> assignedRoomNumbers;

    for (int i = 0; i < roomsNeeded; ++i)
    {
        int rIdx = candidateRoomIndices[i];
        assignedRoomNumbers.push_back(rooms[rIdx].roomNumber);
        double roomTotal = rooms[rIdx].pricePerNight * totalNights;
        grandTotal += roomTotal;

        // Persist record
        file << "Customer: " << guestName
             << " | Room: " << rooms[rIdx].roomNumber
             << " | Type: " << rooms[rIdx].roomType
             << " | Bed: " << rooms[rIdx].bedSetup
             << " | In: " << checkIn.toString()
             << " | Out: " << checkOut.toString()
             << " | Nights: " << totalNights
             << " | Cost: $" << fixed << setprecision(2) << roomTotal << "\n";
    }
    file.close();

    // Step 6: Booking Confirmation Receipt
    cout << "\n=======================================================\n";
    cout << "                BOOKING CONFIRMATION RECEIPT           \n";
    cout << "=======================================================\n";
    cout << "Primary Guest      : " << guestName << "\n";
    cout << "Party Breakdown    : " << adults << " Adult(s), " << children << " Child(ren)\n";
    cout << "Stay Dates         : " << checkIn.toString() << " to " << checkOut.toString()
         << " (" << totalNights << " night" << (totalNights > 1 ? "s" : "") << ")\n";
    cout << "Rooms Reserved     : " << roomsNeeded << " x " << selectedType << " Room\n";
    cout << "Assigned Room #(s) : ";
    for (size_t i = 0; i < assignedRoomNumbers.size(); ++i)
    {
        cout << assignedRoomNumbers[i] << (i + 1 < assignedRoomNumbers.size() ? ", " : "");
    }
    cout << "\nRate Per Room/Night: $" << fixed << setprecision(2) << rooms[candidateRoomIndices[0]].pricePerNight << "\n";
    cout << "Total Amount Due   : $" << grandTotal << "\n";
    cout << "Status             : Confirmed & Saved to bookings.txt\n";
    cout << "=======================================================\n";
}

// Display past bookings from file
void viewAllBookings()
{
    ifstream file("bookings.txt");
    cout << "\n============================== RESERVATION LOG ==============================\n";
    if (!file.is_open())
    {
        cout << "No booking records found.\n";
        return;
    }
    string line;
    int count = 1;
    while (getline(file, line))
    {
        if (!line.empty())
        {
            cout << count++ << ". " << line << "\n";
        }
    }
    if (count == 1)
        cout << "File is empty.\n";
    file.close();
}

int main()
{
    // Hotel Inventory (18 Rooms covering all real-life categories)
    const int TOTAL_ROOMS = 18;
    Room hotelInventory[TOTAL_ROOMS] = {
        // Single Rooms ($30/night, max 1 guest)
        Room(101, "Single", "1 Single Bed", 1, 30.00),
        Room(102, "Single", "1 Single Bed", 1, 30.00),
        Room(103, "Single", "1 Single Bed", 1, 30.00),

        // Double Rooms ($45/night, max 2 guests)
        Room(201, "Double", "1 Double Bed", 2, 45.00),
        Room(202, "Double", "1 Double Bed", 2, 45.00),
        Room(203, "Double", "1 Double Bed", 2, 45.00),

        // Twin Rooms ($50/night, max 2 guests)
        Room(301, "Twin", "2 Twin Beds", 2, 50.00),
        Room(302, "Twin", "2 Twin Beds", 2, 50.00),
        Room(303, "Twin", "2 Twin Beds", 2, 50.00),

        // Deluxe Rooms ($75/night, max 3 guests)
        Room(401, "Deluxe", "1 King Bed", 3, 75.00),
        Room(402, "Deluxe", "1 King Bed", 3, 75.00),
        Room(403, "Deluxe", "1 King Bed", 3, 75.00),

        // Family Rooms ($100/night, max 4 guests)
        Room(501, "Family", "2 Queen Beds", 4, 100.00),
        Room(502, "Family", "2 Queen Beds", 4, 100.00),
        Room(503, "Family", "2 Queen Beds", 4, 100.00),

        // Suite Rooms ($150/night, max 5 guests)
        Room(601, "Suite", "1 King + Sofa Bed", 5, 150.00),
        Room(602, "Suite", "1 King + Sofa Bed", 5, 150.00),
        Room(603, "Suite", "1 King + Sofa Bed", 5, 150.00)};

    int choice = 0;
    do
    {
        cout << "\n============================================\n";
        cout << "         HOTEL RESERVATION SYSTEM           \n";
        cout << "============================================\n";
        cout << "1. Search & Reserve Rooms\n";
        cout << "2. View Room Directory & Base Rates\n";
        cout << "3. View All Booking History\n";
        cout << "4. Exit\n";
        cout << "Enter choice (1-4): ";

        if (!(cin >> choice))
        {
            cout << "Invalid input. Please enter a number.\n";
            clearInput();
            continue;
        }
        clearInput();

        switch (choice)
        {
        case 1:
            bookRoomsFlow(hotelInventory, TOTAL_ROOMS);
            break;
        case 2:
            displayAllRooms(hotelInventory, TOTAL_ROOMS);
            break;
        case 3:
            viewAllBookings();
            break;
        case 4:
            cout << "\nExiting reservation engine. Goodbye!\n";
            break;
        default:
            cout << "Invalid selection. Please choose 1 to 4.\n";
        }
    } while (choice != 4);

    return 0;
}
