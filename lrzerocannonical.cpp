#include <iostream>
#include <vector>
#include <set>
#include <map>

using namespace std;

// Structure to represent an LR(0) item
struct LR0Item {
    char lhs;   // Left-hand side of the production
    string rhs; // Right-hand side of the production

    // Overloading the equality operator for set operations
    bool operator==(const LR0Item& other) const {
        return lhs == other.lhs && rhs == other.rhs;
    }

    // Overloading the less-than operator for set ordering
    bool operator<(const LR0Item& other) const {
        if (lhs != other.lhs) {
            return lhs < other.lhs;
        }
        return rhs < other.rhs;
    }
};

// Function to compute the closure of an LR(0) item
set<LR0Item> computeClosure(const LR0Item& item,  vector<char>& nonTerminals,  vector<vector<string> >& productions) {
    set<LR0Item> closure;
    closure.insert(item);

    vector<LR0Item> stack;
    stack.push_back(item);

    while (!stack.empty()) {
        LR0Item currentItem = stack.back();
        stack.pop_back();

        size_t dotPosition = currentItem.rhs.find('.');
        if (dotPosition != string::npos && dotPosition < currentItem.rhs.length() - 1) {
            char nextSymbol = currentItem.rhs[dotPosition + 1];
            if (find(nonTerminals.begin(), nonTerminals.end(), nextSymbol) != nonTerminals.end()) {
                // Next symbol is a non-terminal, expand it
                for (const vector<string>& production : productions) {
                    if (production[0][0] == nextSymbol) {
                        LR0Item newItem;
                        newItem.lhs = nextSymbol;
                        newItem.rhs = "." + production[1];
                        if (closure.find(newItem) == closure.end()) {
                            closure.insert(newItem);
                            stack.push_back(newItem);
                        }
                    }
                }
            }
        }
    }

    return closure;
}

// Function to compute the goto set for a given set of LR(0) items and a symbol
set<LR0Item> computeGoto(const set<LR0Item>& items, char symbol,  vector<char>& nonTerminals,  vector<vector<string> >& productions) {
    set<LR0Item> goToSet;

    for (const LR0Item& item : items) {
        size_t dotPosition = item.rhs.find('.');
        if (dotPosition != string::npos && dotPosition < item.rhs.length() - 1 && item.rhs[dotPosition + 1] == symbol) {
            LR0Item newItem = item;
            newItem.rhs[dotPosition] = symbol;
            newItem.rhs[dotPosition + 1] = '.';
            goToSet.insert(newItem);
        }
    }

    for (const LR0Item& item : computeClosure(*(goToSet.begin()), nonTerminals, productions)) {
        goToSet.insert(item);
    }

    return goToSet;
}

// A function to compute the first set for a non-terminal symbol
set<char> computeFirstSet( char nonTerminal,  vector<vector<string> >& productions, map<char, set<char> >& firstSets) {
    // Check if we've already computed the first set for this non-terminal
    if (firstSets.find(nonTerminal) != firstSets.end()) {
        return firstSets[nonTerminal];
    }

    // Create an empty set to hold the first symbols
    set<char> first;
    // Iterate through each production rule
    for (auto& rule : productions) {
        if (rule[0][0] == nonTerminal) {

            // If the production rule starts with the non-terminal, add the first symbol of its RHS to the first set
            for(auto& rhs: rule){


                int count = 0;

                char firstSymbol = rhs[count];

                if (isupper(firstSymbol)) {
                    if(firstSymbol != rule[0][0]){
                        bool flag = true;
                        while(flag){
                            set<char> firstSymbolFirstSet = computeFirstSet(firstSymbol, productions, firstSets);
                            if(firstSymbolFirstSet.find('~') != firstSymbolFirstSet.end()){
                                firstSymbolFirstSet.erase('~');
                                count++;
                                firstSymbol = rhs[count];
                            }else{
                                flag = false;
                            }
                            first.insert(firstSymbolFirstSet.begin(), firstSymbolFirstSet.end());       
                        }
                    }
                // If the first symbol is a non-terminal, compute its first set and add it to the first set of the current non-terminal
                    
                } else {
                // If the first symbol is a terminal, add it to the first set of the current non-terminal
                    first.insert(firstSymbol);
                }
            }
            
        }
    }
    // Cache the computed first set for future use
    firstSets[nonTerminal] = first;

    // Return the computed first set
    return first;
}

// A function to compute the follow set for a non-terminal symbol
set<char> computeFollowSet( char nonTerminal,  vector<vector<string> >& productions, map<char, set<char> >& firstSets, map<char, set<char> >& followSets) {
    // Check if we've already computed the follow set for this non-terminal
    if (followSets.find(nonTerminal) != followSets.end()) {
        return followSets[nonTerminal];
    }

    // Create an empty set to hold the follow symbols
    set<char> follow;

    // The start symbol's follow set contains the end-of-input symbol ($)
    if (nonTerminal == 'D') {
        follow.insert('$');
    }

    // Iterate through each production rule
    for (auto& rule : productions) {
        for (int i = 0; i < rule[1].size(); i++) {
            string rhsSymbol = rule[1];
            
            for(int j = 0; j < rhsSymbol.length(); j++){
                if(rhsSymbol[j] == nonTerminal){
                    if(isupper(rhsSymbol[j + 1])){
                        int count = 1;
                        bool flag = true;
                        while(flag){
                            set<char> firstSymbolFirstSet = computeFirstSet(rhsSymbol[j + count], productions, firstSets);
                            if(firstSymbolFirstSet.find('~') != firstSymbolFirstSet.end()){
                                firstSymbolFirstSet.erase('~');
                                count++;
                                follow.insert(firstSymbolFirstSet.begin(), firstSymbolFirstSet.end()); 
                            }else if(rhsSymbol[j + count] == '\0'){
                                set<char> lhsFollowSet = computeFollowSet(rule[0][0], productions, firstSets, followSets);
                                follow.insert(lhsFollowSet.begin(), lhsFollowSet.end());
                                flag = false;
                            }else{
                                follow.insert(firstSymbolFirstSet.begin(), firstSymbolFirstSet.end()); 
                                flag = false;
                            }
                        }
                    }else if(rhsSymbol[j + 1] == '\0'){
                        set<char> lhsFollowSet = computeFollowSet(rule[0][0], productions, firstSets, followSets);
                        follow.insert(lhsFollowSet.begin(), lhsFollowSet.end());
                    }else{
                        follow.insert(rhsSymbol[j+1]);
                    }
                }
            }
        }
    }

    // Cache the computed follow set for future use
    followSets[nonTerminal] = follow;

    // Return the computed follow set
    return follow;
}

// Function to compute the canonical collection of LR(0) items
vector<set<LR0Item> > computeCanonicalCollection( vector<char>& nonTerminals,  vector<char>& terminals,  vector<vector<string> >& productions) {
    vector<set<LR0Item> > canonicalCollection;
    set<LR0Item> initialItem;

    // Create a map to cache the computed first sets
    map<char, set<char> > firstSets;

    // Iterate through each non-terminal symbol and compute its first set
    for (auto& rule : productions) {
        char nonTerminal = rule[0][0];
        computeFirstSet(nonTerminal, productions, firstSets);
    }

    for (auto& entry : firstSets) {
        char nonTerminal = entry.first;
        set<char> firstSet = entry.second;

        cout << "FIRST(" << nonTerminal << ") = {";
        for (auto& symbol : firstSet) {
            cout << symbol << " ";
        }
        cout << "}" << endl;
    }

    map<char, set<char> > followSets;
    
    for (auto& rule : productions) {
        char nonTerminal = rule[0][0];
        computeFollowSet(nonTerminal, productions, firstSets, followSets);
    }


    for (auto& entry : followSets) {
        char nonTerminal = entry.first;
        set<char> followSet = entry.second;

        cout << "FOLLOW(" << nonTerminal << ") = {";
        for (auto& symbol : followSet) {
            cout << symbol << " ";
        }
        cout << "}" << endl;
    }

    // Add the initial item to the canonical collection
    LR0Item item;
    item.lhs = nonTerminals[0];
    item.rhs = "." + productions[0][1];
    initialItem.insert(item);
    initialItem = computeClosure(item, nonTerminals, productions);
    canonicalCollection.push_back(initialItem);

    for (size_t i = 0; i < canonicalCollection.size(); i++){
        set<LR0Item> currentItemSet = canonicalCollection[i];

        // Find all unique symbols following the dot in the current item set
        set<char> symbols;
        for (const LR0Item& item : currentItemSet) {
            size_t dotPosition = item.rhs.find('.');
            if (dotPosition != string::npos && dotPosition < item.rhs.length() - 1) {
                char symbol = item.rhs[dotPosition + 1];
                symbols.insert(symbol);
            }
        }

        // Compute the Goto sets for each symbol and add them to the canonical collection
        for (char symbol : symbols) {
            set<LR0Item> goToSet = computeGoto(currentItemSet, symbol, nonTerminals, productions);

            // Check if the computed Goto set already exists in the canonical collection
            auto it = find(canonicalCollection.begin(), canonicalCollection.end(), goToSet);
            if (it == canonicalCollection.end()) {
                canonicalCollection.push_back(goToSet);
            }
        }
    }

    // Construct the parsing table
    map<pair<int, char>, string> parsingTable;
    map<pair<int, char>, int> gotoTable;

    for (size_t i = 0; i < canonicalCollection.size(); i++) {
        const set<LR0Item>& itemSet = canonicalCollection[i];

        for (const LR0Item& item : itemSet) {
            size_t dotPosition = item.rhs.find('.');
            if (dotPosition != string::npos && dotPosition < item.rhs.length() - 1) {
                char symbol = item.rhs[dotPosition + 1];
                if (find(terminals.begin(), terminals.end(), symbol) != terminals.end()) {
                    // Shift action
                    int nextState = distance(canonicalCollection.begin(), find(canonicalCollection.begin(), canonicalCollection.end(), computeGoto(itemSet, symbol, nonTerminals, productions)));
                    parsingTable[make_pair(i, symbol)] = "Shift " + to_string(nextState);
                } else {
                    // Goto action
                    int nextState = distance(canonicalCollection.begin(), find(canonicalCollection.begin(), canonicalCollection.end(), computeGoto(itemSet, symbol, nonTerminals, productions)));
                    gotoTable[make_pair(i, symbol)] = nextState;
                }
            } else if (item.lhs != 'D') {
                
                vector<string> toSend;

                toSend.push_back(string(1, item.lhs));
                toSend.push_back(item.rhs.substr(0,item.rhs.length()-1));

                cout << toSend[1] << endl;

                cout << item.lhs << endl;
                int productionIndex = distance(productions.begin(), find(productions.begin(), productions.end(), toSend));
                cout << productionIndex << endl;
                
                if (followSets.find(item.lhs) != followSets.end()) {
                    for(auto& follow : followSets[item.lhs]){
                        parsingTable[make_pair(i, follow)] = "Reduce " + to_string(productionIndex);
                    }
                }
                // Reduce action
                
            } else if (item.lhs == 'D' && item.rhs == productions[0][1] + ".") {
                // Accept action
                parsingTable[make_pair(i, '$')] = "Accept";
            }
        }
    }

    // Display the parsing table
    cout << "Parsing Table:" << endl;
    for (const auto& entry : parsingTable) {
        cout << "State " << entry.first.first << ", Symbol " << entry.first.second << " : " << entry.second << endl;
    }

    // Display the goto table
    cout << "Goto Table:" << endl;
    for (const auto& entry : gotoTable) {
        cout << "State " << entry.first.first << ", Symbol " << entry.first.second << " : " << entry.second << endl;

    }

    return canonicalCollection;
}

// Function to display the canonical collection of LR(0) items
void displayCanonicalCollection(const vector<set<LR0Item> >& canonicalCollection) {
    for (size_t i = 0; i < canonicalCollection.size(); i++) {
        cout << "I" << i << ":" << endl;
        for (const LR0Item& item : canonicalCollection[i]) {
            cout << item.lhs << " -> " << item.rhs << endl;
        }
        cout << endl;
    }
}

int main() {
    // Define the grammar
    vector<char> nonTerminals;
    
    nonTerminals.push_back('D');
    nonTerminals.push_back('E');
    nonTerminals.push_back('T');
    nonTerminals.push_back('F');

    vector<char> terminals;
    terminals.push_back('*');
    terminals.push_back('+');
    terminals.push_back('i');
    terminals.push_back('(');
    terminals.push_back(')');

    vector<vector<string> > productions;
    
    vector<string> prodZero;
    vector<string> prodOne;
    vector<string> prodTwo;
    vector<string> prodThree;
    vector<string> prodFour;
    vector<string> prodFive;
    vector<string> prodSix;

    prodZero.push_back("D");
    prodZero.push_back("E");

    prodOne.push_back("E");
    prodOne.push_back("E+T");

    prodTwo.push_back("E");
    prodTwo.push_back("T");

    prodThree.push_back("T");
    prodThree.push_back("T*F");
    
    prodFour.push_back("T");
    prodFour.push_back("F");

    prodFive.push_back("F");
    prodFive.push_back("(E)");

    prodSix.push_back("F");
    prodSix.push_back("i");

    productions.push_back(prodZero);
    productions.push_back(prodOne);
    productions.push_back(prodTwo);
    productions.push_back(prodThree);
    productions.push_back(prodFour);
    productions.push_back(prodFive);
    productions.push_back(prodSix);

    // Compute the canonical collection
    vector<set<LR0Item> > canonicalCollection = computeCanonicalCollection(nonTerminals, terminals, productions);

    // Display the canonical collection
    displayCanonicalCollection(canonicalCollection);

    return 0;
}
