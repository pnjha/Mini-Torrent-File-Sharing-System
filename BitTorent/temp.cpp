fio.open("sample.txt", ios::trunc | ios::out | ios::in); 

// Execute a loop If file successfully Opened 
while (fio) { 

    // Read a Line from standard input 
    getline(cin, line); 

    // Press -1 to exit 
    if (line == "-1") 
        break; 

    // Write line in file 
    fio << line << endl; 
} 

// Execute a loop untill EOF (End of File) 
// point read pointer at beginning of file 
fio.seekg(0, ios::beg); 

while (fio) { 

    // Read a Line from File 
    getline(fio, line); 

    // Print line in Console 
    cout << line << endl; 
} 

// Close the file 
fio.close(); 