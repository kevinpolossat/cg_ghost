#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

using std::queue;
using std::vector;
using std::pair;
using std::priority_queue;

struct Factory {
    int type = -3;
    int nbUnit = -3;
    int prod = -3;
    int balance = 0;
};

struct Troop {
    int type;
    int to;
    int nbUnit;
    int nbTour;
};

vector<vector<pair<int, int>>>      graph;             // DISTANCE GRAPH
vector<Factory *>                   factory;      // SCORES FACTORIES

#include <climits>

int factoryCount;

static bool minHeapComp(pair<int, int> &a, pair<int, int> &b) {
  return a.second > b.second;
}


static void changePriority(vector<pair<int, int>> &q, int which, int dist) {
  int i = 0;
  while (q[i].first != which) {
    ++i;
  }
  q[i].second = dist;
  make_heap(q.begin(), q.end(), minHeapComp);
}

static int shortestPath(vector<int> & prev, int from, int to) {

    while (prev[to] != from) {
        to = prev[to];
    }
    return to;
}

static int pathFinding(int from, int to, int onlyDist) {
    vector<int> dist(factoryCount, INT_MAX);
    vector<pair<int, int>> q(factoryCount);
    vector<int> prev(factoryCount, from);

    int relaxing;
    pair<int, int> curr;

    for (int i = 0; i < factoryCount; ++i) {
       q[i].first = i;
       if (i == from) {
            dist[i] = 0;
            q[i].second = 0;
        }
        else {
            q[i].second = INT_MAX; 
        }
    }
    std::make_heap(q.begin(), q.end(), minHeapComp);
    while (!q.empty()) {
    curr = q.front();
    std::pop_heap(q.begin(), q.end(), minHeapComp);
    q.pop_back();
    if (curr.second != INT_MAX) {
      for (int i = 0; i < graph[curr.first].size(); ++i) {
        relaxing = graph[curr.first][i].first;
        if (dist[relaxing] > dist[curr.first] + graph[curr.first][i].second) {
          dist[relaxing] = curr.second + graph[curr.first][i].second;
          prev[relaxing] = curr.first;
          changePriority(q, relaxing, dist[relaxing]);
        }
      }
    }
  }
  if (onlyDist) {
    return dist[to] == INT_MAX ? -1 : dist[to];
  }
  return dist[to] == INT_MAX ? -1 : shortestPath(prev, from, to);
}


int motherCore;
int enemyCore;
int toControl;
bool printed;
int bombs = 2;
int bombTour = 0;
int nbTour = 0;
int myProd = 0;
int enemyProd = 0;
int avgUnit = 0;

static void display() {
    if (printed) {
        std::cout << ";";
    }
    else {
        printed = true;
    }
}


int bombTarget;

static int bombManagement() {
    int facto = -1;
    for (int y = 0; y < factoryCount; ++y) {
        if (factory[y]->type < 0 &&
            factory[y]->balance < 1 && (facto < 0 || factory[y]->prod > factory[facto]->prod || 
                (factory[y]->prod == factory[facto]->prod && factory[y]->nbUnit > factory[facto]->nbUnit))) {
            facto = y;
        }
    } // THROW FROM CLOSEST...
    if (facto >= 0) {
        display();
        int minDist;
        int from = -1;
        for (int i = 0; i < graph[facto].size(); ++i) { // SEARCH FOR CLOSEST TO LAUNCH BOMB.
            if (factory[graph[facto][i].first]->type == 1 &&
                (from < 0 || graph[facto][i].second < minDist)) {
                from = graph[facto][i].first;
                minDist = graph[facto][i].second;
            }
        }
        std::cout << "BOMB " << from << " " << facto;
        --bombs;
        bombTour = 10;
        cerr <<"["<< bombTour << " " << graph[from][facto].second << "]"<< endl;;
        return 1;
    }
    return 0;
}

static int balanceTroop(   int a,
                    std::vector<Troop> &troops) {
    int attacking = factory[a]->type < 1 ? -factory[a]->nbUnit : factory[a]->nbUnit;
    for (auto & t : troops) {
        if (t.to == a) {
            attacking += t.type < 1 ? -t.nbUnit : t.nbUnit;
        }
    }
    return attacking;
}

static void computeBalance(vector<Troop> & troops) {
    for (int i = 0; i < factoryCount; ++i) {
        factory[i]->balance = balanceTroop(i, troops);
        cerr << i << " with balance " << factory[i]->balance << endl;
    }
}

static inline float evaluateScore(  float prod, 
                                    float dist,
                                    float nbUnit) {
    return prod / dist / nbUnit; // ?? (nbUnit + 1 != 0 && dist > 0); CHANGE TO NBUNIT / PROD / DIST
}

static void expand() {
    sort(graph[motherCore].begin(), graph[motherCore].end(), [&](pair<int, int> &a, pair<int, int> &b){
        return  a.second < b.second || 
                (a.second == b.second && factory[a.first]->prod > factory[b.first]->prod)/*evaluateScore(  factory[a.first]->prod,
                                a.second,
                                factory[a.first]->nbUnit + 1) > 
                evaluateScore(  factory[b.first]->prod,
                                b.second,
                                factory[b.first]->nbUnit + 1)*/;
    });
    for (int i = 0; i < graph[motherCore].size(); ++i) {
        if (factory[graph[motherCore][i].first]->prod && 
            factory[motherCore]->nbUnit > factory[graph[motherCore][i].first]->nbUnit
            ) {
            int toGo = pathFinding(motherCore, graph[motherCore][i].first, 0);
            display();
            cout << "MOVE " << motherCore << " " <<
            toGo << " " <<
            factory[graph[motherCore][i].first]->nbUnit + 1;
            cerr << "EXPAND : " << motherCore << " to " << graph[motherCore][i].first << " by " << toGo << endl;
            factory[motherCore]->nbUnit -= factory[graph[motherCore][i].first]->nbUnit + 1;
        }
    }
}

static int maxScore(int from) {
    float maxScore;
    float retNeighbor = -1;
    int score;
    for (int neighbor = 0; neighbor < graph[from].size(); ++neighbor) {
        if (factory[graph[from][neighbor].first]->type < 1 && 
            factory[graph[from][neighbor].first]->prod &&
            (factory[graph[from][neighbor].first]->type != 0 || factory[graph[from][neighbor].first]->balance < 1)) {
            score = pathFinding(from, graph[from][neighbor].first, 1);/*graph[from][neighbor].second*//*evaluateScore(  factory[graph[from][neighbor].first]->prod,
                                    graph[from][neighbor].second,
                                    factory[graph[from][neighbor].first]->nbUnit + 1)*/;
            //cerr << from << " to " << graph[from][neighbor].first << " score : " << score << endl;
            if (retNeighbor < 0 || (score < maxScore || 
                (score == maxScore && factory[graph[from][neighbor].first]->balance > factory[retNeighbor]->balance))) {
                retNeighbor = graph[from][neighbor].first;
                maxScore = score;
            }
        }
    }
    return retNeighbor;
}

static void act() {
    int maxSc;
    for (int i = 0; i < factoryCount; ++i) {
        maxSc = maxScore(i);
        cerr << i << " to "  << maxSc << endl;
        if (factory[i]->type == 1 && factory[i]->nbUnit > 0 && factory[i]->balance > 0 && maxSc > -1) {
            int toGo = pathFinding(i, maxSc, 0);
            cerr << "CHECK : " << factory[toGo]->type << " " << factory[toGo]->balance << endl;
            if (!factory[i]->prod || factory[i]->nbUnit > factory[maxSc]->nbUnit || factory[toGo]->type == 1) {
                display();
                if (factory[i]->prod) {
                    cout << "MOVE " << i << " " << toGo << " " << 
                    (factory[i]->nbUnit > factory[i]->balance ? factory[i]->balance - 1 : factory[i]->nbUnit);
                }
                else {
                     cout << "MOVE " << i << " " << toGo << " " << factory[i]->nbUnit;   
                }
            }
        }
    }
}

int main() {

    cin >> factoryCount; cin.ignore();
    toControl = factoryCount / 2 + 1;
    int linkCount; // the number of links between factories
    cin >> linkCount; cin.ignore();
    Factory *f;
    for (int i = 0; i < factoryCount; ++i) {
        f = new Factory;
        factory.push_back(f);
    }

    graph.resize(factoryCount);
    
    for (int i = 0; i < linkCount; i++) {
        int factory1;
        int factory2;
        int distance;
        cin >> factory1 >> factory2 >> distance; cin.ignore();
        cerr <<" GRAPH "<< factory1 <<  " to " << factory2 << " " << distance << endl; 
        graph[factory1].push_back(make_pair(factory2, distance));
        graph[factory2].push_back(make_pair(factory1, distance));
    }

    // GENERATE ALL POSSIBLE SPLITS;
    bool first = true;
    int toSend = (30 - 1) / toControl;
    int c;
    int oneFactoToBomb = false;
    bool expandPhase = true;
    while (1) {
        int entityCount;// the number of entities (e.g. factories and troops)
        bool f = 0; // IF A SOLUTION IS FOUND // SOLUTIONS
        pair<int, int> solution(-1, -1);
        int dist = -1;
        
        vector<Troop> troops;

        cin >> entityCount; cin.ignore();
        enemyCore = -1;
        c = 0;
        motherCore = -1;
        myProd = 0;
        enemyProd = 0;
        avgUnit = 0;
        for (int i = 0; i < entityCount; i++) {
            int entityId;
            string entityType;
            int arg1;
            int arg2;
            int arg3;
            int arg4;
            int arg5;
            cin >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4 >> arg5; cin.ignore();
            if (entityType == "FACTORY") {
                if (arg1 == 1) {
                    myProd += arg3;
                }
                else if (arg1 < 0) {
                    enemyProd += arg3;
                }
                if (arg1 == 1 && (  motherCore < 0 )) {
                    motherCore = entityId; // SETUP MOTHER CORE...
                }
                else if (arg1 < 0 && (enemyCore < 0)) {
                    enemyCore = entityId;

                }
                else if (arg1 < 0 && arg3 >= 2) {
                    oneFactoToBomb = true;
                }
                if (arg1 == 1) {
                    ++c;
                }
                factory[entityId]->type  = arg1;
                factory[entityId]->nbUnit = arg2;
                factory[entityId]->prod = arg3;
                avgUnit += arg2;
//                std::cerr << "{" << entityId << " " << factory[entityId]->type << "}" << std::endl;

            }
            else if (entityType == "TROOP") { // IF TROOP ARRIVED 
                Troop t;
                t.type = arg1;
                t.to = arg3;
                t.nbUnit = arg4;
                t.nbTour = arg5;
                troops.push_back(t);
            }
            else { // BOMB

            }
        }
        printed = false;

        avgUnit /= factoryCount;
        sort(troops.begin(), troops.end(), [](Troop const & a, Troop const & b) {
            return a.nbTour < b.nbTour;
        });
        display();
        std::cout << "MSG MY PROD " << myProd << " ENEMY PROD " << enemyProd;
        if (!expandPhase) {
            computeBalance(troops);
            if (bombs && bombTour <= 0 && oneFactoToBomb) {
                bombManagement();
            }
            act();
        }
        else {
            expandPhase = false;
            expand();
        }
        if (!printed) {
            std::cout << "WAIT" << std::endl;
        }
        else {
            std::cout << std::endl;
        }
        std::cerr << "MOTHER CORE :: " << motherCore << std::endl;
        --bombTour;
        ++nbTour;
    }
}


