#include "LeynaDM.h"

using namespace std;

struct row_counter {
    int pos;
    int count;
};

struct HitNote {
    int row;
    float hitTime;
    bool isHold;
    float holdTime = 0;
};

struct Map {
    vector < HitNote > notes;
    int RowCount = 1; //<- DevisionByZero catch
};

Map generateMap(int notes, int rows) {
    Map rt;
    rt.RowCount = rows;
    for (int i = 0; i < notes; i++) {
        HitNote note;
        note.hitTime = (float)i;
        note.row = i % rows;
        rt.notes.push_back(note);
    }

    return rt;
}

Map readMap(string path) {
    path = regex_replace(path, regex("\\\""), "");
    Map rt;
    ifstream infile(path);
    string line;

    bool hitobj = false;

    while (getline(infile, line)) {
        istringstream iss(line);
        string a;
        iss >> a;

        if (hitobj) {

            HitNote hn;

            stringstream ss(a);
            string seg;
            vector < string > segList;
            while (getline(ss, seg, ',')) {
                segList.push_back(seg);
            }
            int x = stoi(segList[0]);
            float time = stof(segList[2]);

            if (stoi(segList[3]) == 128) {
                hn.isHold = true;
                hn.holdTime = stof(segList[5]);
            }

            hn.row = x;
            hn.hitTime = time;
            rt.notes.push_back(hn);
        }
        else if (a == "[HitObjects]") {
            hitobj = true;
        }
        else if (a.starts_with("CircleSize")) {
            stringstream ss(a);
            string seg;
            vector < string > segList;
            while (getline(ss, seg, ':')) {
                segList.push_back(seg);
            }
            int LvlRow = stoi(segList[1]);
            rt.RowCount = LvlRow;
        }

    }
    return rt;
}

struct Pointer {
    int position = 0;
    bool active = false;
    int totalDistance = 0;
    float totalDifficulty = 0;
    int usages = 0;
    float holdUntil = 0;
};

int checkforIdlePointer(vector < Pointer > pList) {
    for (int i = 0; i < pList.size(); i++) {
        if (pList[i].active == false) {
            return i;
        }
    }
    return -1;
}

int diffcalc() {
    float densityFactor = 16.0 f;
    float distanceFactor = 1.0 f;
    float star_factor = 0.0084 f;

    string line;
    getline(cin, line);
    Map stage = readMap(line);
    vector < row_counter > rows;

    for (HitNote note : stage.notes) {
        bool added = false;
        for (row_counter& rc : rows) {
            if (note.row == rc.pos) {
                rc.count++;
                added = true;
                break;
            }
        }
        if (!added) {
            row_counter rc;
            rc.pos = note.row;
            rc.count = 1;
            rows.push_back(rc);
        }
    }
    for (HitNote& note : stage.notes) {
        for (int i = 0; i < rows.size(); i++) {
            if (note.row == rows[i].pos) {
                note.row = i;
            }
        }
    }
    vector < Pointer > pointers;

    //Make sure to create a pointer for the first note
    float currentHitTime = stage.notes[0].hitTime;
    int concurrentNotes = 1;
    //Iterate over all notes
    for (int index = 0; index < stage.notes.size(); index++) {
        auto note = stage.notes[index];

        //Make sure there are enough pointers (Multiple Notes at once)
        if (currentHitTime == note.hitTime) {
            concurrentNotes++;
            while (concurrentNotes > pointers.size() || pointers.size() < stage.RowCount + 1) {
                Pointer p;
                p.active = false;
                p.position = note.row;
                p.totalDistance = 0;
                pointers.push_back(p);
            }
        }
        //Disable all pointers but ln
        else {
            currentHitTime = note.hitTime;
            concurrentNotes = 1;
            for (Pointer& p : pointers) {
                if (p.holdUntil > currentHitTime) continue;
                p.active = false;
            }
        }
        //Iterate through all active pointers and check which one is the nearest
        int minDist = stage.RowCount;
        int pointerIndex = -1;
        for (int i = 0; i < pointers.size(); i++) {
            Pointer p = pointers[i];
            if (!p.active) {
                int pDist = (int)abs(p.position - note.row) % (int)ceil(stage.RowCount / 2);
                if (pDist < minDist) {
                    minDist = pDist;
                    pointerIndex = i;
                }
            }
        }
        if (pointerIndex == -1) {
            cout << "-5\n";
            return -5;
        }

        float LNBonus = 1;
        if (note.isHold) {
            LNBonus = (note.holdTime - currentHitTime) / stage.notes[stage.notes.size() - 1].hitTime;
            pointers[pointerIndex].holdUntil = note.holdTime;
        }

        //Let the nearest pointer go
        pointers[pointerIndex].totalDistance += minDist;
        pointers[pointerIndex].position = note.row;
        pointers[pointerIndex].active = true;
        pointers[pointerIndex].usages++;

        //Ignore first note
        if (index > 0)
            pointers[pointerIndex].totalDifficulty += LNBonus / (abs(note.hitTime - stage.notes[index - 1].hitTime) + 1);
    }
    //Calculate Pattern-Difficulty
    float pDiff = 0;
    float sumTop = 0;
    float sumBottom = 0;
    for (Pointer p : pointers) {
        sumTop += p.usages * p.totalDifficulty;
        sumBottom += p.usages;
        cout << p.totalDifficulty << "\t|\t" << p.usages << "\n";
    }
    pDiff = (sumTop / sumBottom);
    cout << "Pattern: " << pDiff << "\n";

    //Calculate Density-Difficulty
    float totalTimeDelta = 0.001 * (stage.notes[stage.notes.size() - 1].hitTime - stage.notes[0].hitTime);

    float Density = stage.notes.size() / totalTimeDelta;

    cout << "Density: " << Density << "\n";

    float difficulty = 10 * star_factor * pow((pDiff * Density) / (pDiff + Density), 2) / stage.RowCount;

    cout << "Difficulty: " << difficulty << "\n";

    return difficulty;
}

int main() {
    while (1) {
        diffcalc();
    }
}