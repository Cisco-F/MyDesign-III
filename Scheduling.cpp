# include "Scheduling.h"
# include "Hu.h"
# include "MR_LCS.h"
# include "ASAP.h"
# include "ALAP.h"

void menu()
{
    int opt;
    string filepath;
    while (true) {
        cout << "****************************************" << endl;
        cout << "*             Scheduling Tool          *" << endl;
        cout << "****************************************" << endl;
        cout << "*             1. select file           *" << endl;
        cout << "*             2. exit                  *" << endl;
        cout << "****************************************" << endl;

        cin >> opt;
        system("cls");
        switch (opt) {
        case 1: {
            filepath = select_file();
            cout << filepath << endl;
            Schedule_menu(filepath);
            break;
        }
        case 2: {
            cout << "Thanks for using!" << endl;
            exit(0);
        }
        default: {
            cout << "Invalid input!" << endl;
        }
        }
    }
}

string select_file()
{
    OPENFILENAME ofn;
    WCHAR szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        std::wstring wideFilePath = ofn.lpstrFile;
        std::string filepath(wideFilePath.begin(), wideFilePath.end());
        return filepath;
    }
    else {
        MessageBox(NULL, L"未选择任何文件", L"提示", MB_OK);
        return "\0";
    }
}

void Schedule_menu(string filepath)
{
    int opt;
    while (true) {
        cout << "****************************************" << endl;
        cout << "              Scheduling Tool           " << endl;
        cout << "****************************************" << endl;
      //cout << "*             1. run all methods       *" << endl;
        cout << "*             1. ASAP Scheduling       *" << endl;
        cout << "*             2. ALAP Scheduling       *" << endl;
        cout << "*             3. Hu Scheduling         *" << endl;
        cout << "*             5. MR_LCS                *" << endl;
        cout << "*             6. exit                  *" << endl;
        cout << "****************************************" << endl;

        cin >> opt;
        system("cls");
        switch (opt) {
                //case 1: {
                //  ASAP_Scheduling(filepath);
                //  ALAP_Scheduling(filepath);
                //  Hu_Scheduling(filepath);
                //  MR_LCS_Scheduling(filepath);
                //  break;
                //}
        case 1: {
                ASAP_Scheduling(filepath);
                break;
            }
        case 2: {
            ALAP_Scheduling(filepath);
            break;
        }
        case 3: {
            Hu_Scheduling(filepath);
            break;
        }
        case 5: {
            MR_LCS_Scheduling(filepath);
            break;
        }
        case 6: {
            cout << "Thanks for using!" << endl;
            exit(0);
        }
        default: {
            cout << "Invalid input!" << endl;
        }
        }
        }
    }