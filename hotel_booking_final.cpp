// Group PROJECT "Hotel Booking System"
#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>
#include <fstream>
using namespace std;

// Array for the room types and prices
const string roomTypes[5] = {"Single", "Double", "Twin", "Deluxe", "Suite"};
const int roomPrices[5] = {50, 80, 130, 200, 220};
const int MAX_ITEMS = 10;                                                              // most room choices in one booking
const string monthNames[12] = {"January", "February", "March", "April", "May", "June", // Declaring months name
                               "July", "August", "September", "October", "November", "December"};
const int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // declaring the total days of each month

// Array for the room booking
string customerNames[100];        // Array to store customer names for each room type
string customerPhoneNumbers[100]; // Array to store customer phone numbers for each room type
string customerEmails[100];       // Array to store customer email addresses for each room type
int bookedTypes[100][MAX_ITEMS];  // Array to store the booked room types for each customer
int nightsBooked[100][MAX_ITEMS]; // Array to store the number of nights booked for each customer
int checkInOrdinal[100];
int totalBookings = 0; // Variable to keep track of the total number of bookings made
int totalRevenue = 0;
int itemCounts[100] = {0}; // Variable to keep track of the total revenue generated from bookings
int totalCosts[100];       // Variable to keep track of the total cost of a booking
int totalCost = 0;

// Array for tracking ehich booking have been cancelled
bool bookingCancelled[100] = {false};

int main()
{
    int choice = 0;
    int todayOrdinal = 0;

    // Load saved bookings from the file (if the file exists)
    ifstream inFile("bookings.txt");
    if (inFile)
    {
        inFile >> totalBookings >> totalRevenue;
        inFile.ignore(10000, '\n'); // skip the rest of the first line

        if (totalBookings < 0 || totalBookings > 100)
        {
            totalBookings = 0; // the file looks damaged, so start empty
        }

        for (int i = 0; i < totalBookings; i++)
        {
            getline(inFile, customerNames[i]);
            getline(inFile, customerPhoneNumbers[i]);
            getline(inFile, customerEmails[i]);

            // Number of rooms, then the room type of each room
            inFile >> itemCounts[i];
            for (int j = 0; j < itemCounts[i]; j++)
            {
                inFile >> bookedTypes[i][j];
            }

            // Nights, check-in day number, total cost, cancelled (1) or active (0)
            int nights;
            inFile >> nights >> checkInOrdinal[i] >> totalCosts[i] >> bookingCancelled[i];
            inFile.ignore(10000, '\n'); // skip the rest of this line

            // Every room in a booking has the same nights
            for (int j = 0; j < itemCounts[i]; j++)
            {
                nightsBooked[i][j] = nights;
            }
        }
        inFile.close();
    }

    do // using do-while loop to display the menu and get user input until the user chooses to exit the program.
    {
        // Calculation Date
        time_t now = time(0);
        tm *localTime = localtime(&now);
        int todayDay = localTime->tm_mday;
        int todayMonth = localTime->tm_mon + 1;
        int todayYear = localTime->tm_year + 1900;

        todayOrdinal = (todayYear - 1) * 365 + (todayYear - 1) / 4 - (todayYear - 1) / 100 + (todayYear - 1) / 400;
        for (int m = 1; m < todayMonth; m++)
        {
            todayOrdinal += daysInMonth[m - 1];
        }
        if (todayMonth > 2 && (todayYear % 400 == 0 || (todayYear % 4 == 0 && todayYear % 100 != 0)))
        {
            todayOrdinal++;
        }
        todayOrdinal += todayDay;

        // Menu
        cout << "==============================\n";
        cout << "   WELCOME TO GRAND HOTEL\n";
        cout << "==============================\n";
        cout << "1. View Room Inventory\n";
        cout << "2. Book a Room\n";
        cout << "3. View Booking History\n";
        cout << "4. Cancel Booking / Request Refund\n";
        cout << "5. Exit\n";
        cout << "==============================\n";
        cout << "Enter your choice: ";
        cin >> choice;
        while (cin.fail())
        {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Please enter a number: ";
            cin >> choice;
        }

        switch (choice)
        {
        case 1:
        { // Room Inventor
            cout << "Room Inventory: \n";
            for (int i = 0; i < 5; i++) // using FOR Loop to display the room types and prices without repeating the code.
            // i = 0; i < 5; i++ means that the loop will start with i equal to 0 and will continue to run as long as i is less than 5.
            // After each iteration of the loop, i will be incremented by 1. This allows the loop to iterate through all the elements in the roomTypes and roomPrices arrays, which have a size of 5.
            {
                cout << "Room Type: " << roomTypes[i] << " - " << roomPrices[i] << "$" << " per night\n";
            }
            break;
        }

        case 2: // Room Booking and Payment
        {
            string name, phoneNumber, email;
            int roomType, nights;
            cout << "\n===== BOOK A ROOM =====\n";

            // Name
            cout << "Enter your name: ";
            cin.ignore(10000, '\n'); // Clear the input buffer to avoid issues with getline after cin
            getline(cin, name);
            while (name.empty())
            {
                cout << "Name cannot be empty. Please try again: ";
                getline(cin, name);
            }

            // Phone Number
            cout << "Enter your phone number: ";
            getline(cin, phoneNumber);
            while (phoneNumber.length() != 9 && phoneNumber.length() != 10)
            {
                if (phoneNumber.empty())
                {
                    cout << "Phone number cannot be empty. Please try again: ";
                }
                else
                {
                    cout << "Invalid phone number. Please enter a 10-digit phone number: ";
                }
                getline(cin, phoneNumber);
            }
            // Email (optional)
            cout << "Enter your email (optional): ";
            getline(cin, email);

            // Enter the Room Type and Number of Nights
            totalCost = 0;
            roomType = -1;         // any number except 0, so the loop starts
            int pricePerNight = 0; // price of all choosen room per night

            // Show the room types
            cout << "\n===== Room Types ======\n";
            for (int i = 0; i < 5; i++)
            {
                cout << i + 1 << ". " << roomTypes[i] << " - " << roomPrices[i] << "$" << " per night\n";
            }

            // keep asking until the customer enters 0
            while (roomType != 0 && itemCounts[totalBookings] < MAX_ITEMS)
            {
                cout << "\nEnter the room type (1-5, or 0 to finish): ";
                cin >> roomType;
                while (cin.fail() || roomType < 0 || roomType > 5)
                {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid room type. Please enter a number between 0 and 5: ";
                    cin >> roomType;
                }

                if (roomType == 0 && itemCounts[totalBookings] == 0)
                {
                    cout << "Please choose at least one room.\n";
                    roomType = -1;
                }
                else if (roomType != 0)
                {
                    // Store the room type (the nights come later)
                    bookedTypes[totalBookings][itemCounts[totalBookings]] = roomType - 1;
                    itemCounts[totalBookings]++;
                    pricePerNight += roomPrices[roomType - 1];

                    cout << "Added: " << roomTypes[roomType - 1] << " \n";
                }
            }

            // Ask the customer for Nights by selecting from the calendar
            cout << "\nToday's Date: " << todayDay << "/" << todayMonth << "/" << todayYear << "\n";

            //
            int date[2][3]; // date[0] means check in, date[1] means checkout
            int ordinal[2]; // the day of each date

            // Using for loops to let it run twice for checkin and checkout
            for (int c = 0; c < 2; c++)
            {
                string label = (c == 0) ? "Check-in" : "Check-Out";

                // Dates up to this day number are unavailable
                int minOrdinal = (c == 0) ? todayOrdinal - 1 : ordinal[0];
                bool dateChosen = false;
                while (!dateChosen)
                {
                    int day, month, year;

                    // Ask them to input the month and year
                    cout << "\nPlease select " << label << " month (1-12): ";
                    cin >> month;
                    while (cin.fail())
                    {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "Please enter a number of the month: ";
                        cin >> month;
                    }
                    cout << "Enter year: ";
                    cin >> year;
                    while (cin.fail())
                    {
                        cin.clear();
                        cin.ignore(1000, '\n');
                        cout << "Please enter a number: ";
                        cin >> year;
                    }

                    // In case the customer enter the wrong month and year
                    if (month < 1 || month > 12 || year < todayYear)
                    {
                        cout << "Invalid. Please choose a current or future month.\n";
                        continue;
                    }
                    if (year == todayYear && month < todayMonth)
                    {
                        cout << "That month is in the past. Please choose another month.\n";
                        continue;
                    }
                    bool leap = (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0));
                    int numberOfDays = daysInMonth[month - 1];
                    if (month == 2 && leap)
                    {
                        numberOfDays = 29;
                    }
                    // Day number of the day BEFORE the 1st of this month
                    int monthStart = (year - 1) * 365 + (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400;
                    for (int m = 1; m < month; m++)
                    {
                        monthStart += daysInMonth[m - 1];
                    }
                    if (month > 2 && leap)
                    {
                        monthStart++;
                    }

                    if (monthStart + numberOfDays <= minOrdinal)
                    {
                        cout << "There are no available dates in that month.\n";
                        continue;
                    }

                    // ----- Show the calendar -----
                    cout << "\n====== " << monthNames[month - 1] << " " << year << " ======\n";
                    cout << "Sun  Mon  Tue  Wed  Thu  Fri  Sat\n";

                    // Find which weekday the 1st falls on
                    tm dateInfo = {};
                    dateInfo.tm_year = year - 1900;
                    dateInfo.tm_mon = month - 1;
                    dateInfo.tm_mday = 1;
                    dateInfo.tm_hour = 12;
                    mktime(&dateInfo);
                    int firstDay = dateInfo.tm_wday;

                    for (int space = 0; space < firstDay; space++)
                    {
                        cout << "     ";
                    }
                    for (int d = 1; d <= numberOfDays; d++)
                    {
                        if (monthStart + d <= minOrdinal)
                        {
                            cout << setw(3) << "--" << "  ";
                        }
                        else
                        {
                            cout << setw(3) << d << "  ";
                        }
                        if ((firstDay + d) % 7 == 0)
                        {
                            cout << "\n";
                        }
                    }
                    cout << "\n-- means the date is unavailable.\n";

                    cout << "Enter " << label << " day: ";
                    cin >> day;
                    while (cin.fail())
                    {
                        cin.clear();
                        cin.ignore(1000, '\n');
                        cout << "Please enter a number: ";
                        cin >> day;
                    }

                    if (day < 1 || day > numberOfDays)
                    {
                        cout << "Invalid day. Please try again.\n";
                        continue;
                    }
                    if (monthStart + day <= minOrdinal)
                    {
                        cout << "That date is unavailable. Please choose another date.\n";
                        continue;
                    }

                    date[c][0] = day;
                    date[c][1] = month;
                    date[c][2] = year;
                    ordinal[c] = monthStart + day;
                    dateChosen = true;
                }
            }

            nights = ordinal[1] - ordinal[0]; // nights = check-out day number - check-in day number
            cout << "\nCheck-in:  " << date[0][0] << "/" << date[0][1] << "/" << date[0][2] << "\n";
            cout << "Check-out: " << date[1][0] << "/" << date[1][1] << "/" << date[1][2] << "\n";
            cout << "Number of nights: " << nights << "\n";
            totalCost = pricePerNight * nights;

            // save the nights for each room of the bookings
            for (int r = 0; r < itemCounts[totalBookings]; r++)
            {
                nightsBooked[totalBookings][r] = nights;
            }

            // Payment: Customer must pay 50% of the deposit before the confirmation of the Bookings
            cout << "\n===== PAYMENT =====\n";
            cout << "Total to pay: " << totalCost << "$\n";
            cout << "Scan the KHQR code to pay " << totalCost << "$\n";
            cout << "Press Enter after you have paid.";
            cin.ignore(10000, '\n');
            cin.get();
            cout << "Payment Received.\n";

            // Store the booking information in the arrays
            customerNames[totalBookings] = name;
            customerPhoneNumbers[totalBookings] = phoneNumber;
            customerEmails[totalBookings] = email;
            totalCosts[totalBookings] = totalCost;
            bookingCancelled[totalBookings] = false;    // Mark the new booking as active
            checkInOrdinal[totalBookings] = ordinal[0]; // Save the check-in date ordinal
            totalBookings++;
            totalRevenue += totalCost;

            // Print Out the booking confirmation and total cost to the user
            cout << "Booking successful! Total cost: " << totalCost << "$\n";
            cout << "Thank you for booking with us, " << name << "!\n";

            break;
            // The cin.ignore(10000, '\n'); cin.get(); pair makes the program wait for Enter, so it feels like a real payment step.
            // The ignore removes the leftover Enter from your last typed number, and cin.get() waits for a new one.
            // Nothing else is needed, because the whole amount is paid online.
        }

        case 3: // View Booking History
        {
            cout << "\n===== BOOKING HISTORY =====\n";

            // Check whether any bookings have been made
            if (totalBookings == 0)
            {
                cout << "No booking was found.\n";
            }
            else
            {
                // Loop through all the bookings
                for (int i = 0; i < totalBookings; i++)
                {
                    cout << "\nBooking #" << i + 1 << endl;

                    // Display the customer's information
                    cout << "Customer Name: " << customerNames[i] << endl;
                    cout << "Phone Number: " << customerPhoneNumbers[i] << endl;
                    if (customerEmails[i] != "")
                    {
                        cout << "Email: " << customerEmails[i] << endl;
                    }

                    // Display all rooms in this booking
                    // Display the rooms in this booking, grouped by room type
                    for (int t = 0; t < 5; t++)
                    {
                        int count = 0;

                        // Count how many rooms of this type are in the booking
                        for (int j = 0; j < itemCounts[i]; j++)
                        {
                            if (bookedTypes[i][j] == t)
                            {
                                count++;
                            }
                        }

                        // Only show room types the customer actually booked
                        if (count > 0)
                        {
                            cout << "Room Type: " << roomTypes[t] << " x" << count << endl;
                        }
                    }

                    // The nights are the same for every room, so show them once
                    cout << "Number of Nights: " << nightsBooked[i][0] << endl;

                    // Display the total cost of this booking
                    cout << "Total Cost: $" << totalCosts[i] << endl;

                    // Display the booking status
                    if (bookingCancelled[i])
                    {
                        cout << "Status: Cancelled\n";
                    }
                    else
                    {
                        cout << "Status: Active\n";
                    }

                    cout << "------------------------------\n";
                }
            }
            break;
        }

        case 4: // Cancel Booking / Request Refund
        {
            cout << "\n===== CANCEL BOOKING / REQUEST REFUND =====\n";

            // Check whether any bookings have been made
            if (totalBookings == 0)
            {
                cout << "No bookings available to cancel.\n";
                break; // Exit Case 4 and return to the main menu
            }

            // Display all bookings so the customer can choose one
            cout << "\n===== BOOKING LIST =====\n";

            for (int i = 0; i < totalBookings; i++)
            {
                // Display the booking number and customer name
                cout << "Booking #" << i + 1 << " | Name: " << customerNames[i];

                // Check whether this booking has already been cancelled
                if (bookingCancelled[i])
                {
                    cout << " | Status: Cancelled\n";
                }
                else
                {
                    cout << " | Status: Active\n";
                }
            }

            // Ask the customer which booking they want to cancel
            int bookingNumber;
            cout << "\nEnter the booking number to cancel: ";
            cin >> bookingNumber;
            while (cin.fail())
            {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Please enter a number: ";
                cin >> bookingNumber;
            }

            // Check whether the entered booking number is valid
            if (bookingNumber < 1 || bookingNumber > totalBookings)
            {
                cout << "Invalid booking number.\n";
                break; // Stop cancellation because the number is invalid
            }

            /* Convert the customer's booking number to the array position.
            Customers count from 1, but array positions start from 0,
            so subtract 1 to access the correct booking. */
            int index = bookingNumber - 1;

            // Check whether the selected booking was already cancelled
            if (bookingCancelled[index])
            {
                cout << "This booking has already been cancelled.\n";
                break; // Prevent cancelling the same booking twice
            }

            // Ask how many days are left before check-in
            int daysBeforeCheckIn = checkInOrdinal[index] - todayOrdinal;

            // Calculate the refund based on the cancellation policy
            int refundAmount = 0;

            if (daysBeforeCheckIn >= 1)
            {
                // Full refund if cancelled at least one day before check-in
                refundAmount = totalCosts[index];
                cout << "You are eligible for a 100% refund.\n";
            }
            else if (daysBeforeCheckIn == 0)
            {
                // 50% refund if cancelled on the check-in day
                refundAmount = totalCosts[index] * 50 / 100;
                cout << "You are eligible for a 50% refund.\n";
            }
            else
            {
                // No refund if cancelled after the check-in day
                refundAmount = 0;
                cout << "Sorry, no refund is available.\n";
            }

            // Mark the selected booking as cancelled
            bookingCancelled[index] = true;

            // Subtract the refund amount from the hotel's total revenue
            totalRevenue -= refundAmount;

            // Display the cancellation confirmation and refund amount
            cout << "\nBooking cancelled successfully\n";
            cout << "Customer: " << customerNames[index] << endl;
            cout << "Refund amount: $" << refundAmount << endl;

            // Inform the customer that the refund is simulated but only if there is a refund
            if (refundAmount > 0)
            {
                cout << "Please note: your refund may take up to 1 day to be sent back to your account.\n";
            }
            else
            {
                cout << "No refund is processed as the check-in date has passed.\n";
            }

            break; // Exit Case 4 and return to the main menu
        }

        case 5: // Exit the program
        {
            // Save all bookings to the file
            ofstream outFile("bookings.txt");
            outFile << totalBookings << " " << totalRevenue << "\n";
            for (int i = 0; i < totalBookings; i++)
            {
                outFile << customerNames[i] << "\n";
                outFile << customerPhoneNumbers[i] << "\n";
                outFile << customerEmails[i] << "\n";

                // Number of rooms, then the room type of each room
                outFile << itemCounts[i];
                for (int j = 0; j < itemCounts[i]; j++)
                {
                    outFile << " " << bookedTypes[i][j];
                }
                outFile << "\n";

                // Nights, check-in day number, total cost, cancelled (1) or active (0)
                outFile << nightsBooked[i][0] << " " << checkInOrdinal[i] << " "
                        << totalCosts[i] << " " << bookingCancelled[i] << "\n";
            }
            outFile.close();
            cout << "Bookings saved to bookings.txt\n";

            // Display a thank you message
            cout << "Thank you for visiting Grand Hotel!\n";
            cout << "Have a nice day!\n";

            break; // Exit the switch statement
        }

        default:
            cout << "Invalid choice. Please try again.\n";
            break;
        }
    } while (choice != 5);

    return 0;
}