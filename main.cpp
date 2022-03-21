//======================================================
// MMsieve - prime generator boolean

// Copyright [2022] [mgr inz. Marek Matusiak]
/* Copyright [2022] Aleksander Starostka

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//======================================================


#include <ctime>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream> 
#include <utility>
#include <iostream>
#include <stdexcept>
#include <algorithm>


/**
 * Some Refactorings, introduced multithreading, cmdline utils aka mini wizard 
 * 21.03.2022 by #cx3#   The main achievement: in 3583 ms all primes from 2 to 10^9  ..(^^,)..
 */


using namespace std;
using namespace std::this_thread;
using namespace std::chrono_literals;
using std::chrono::system_clock;


typedef unsigned int uint;


bool one_of(string s, initializer_list<string> il) {
    /**
     * Tells if value s exists in collection passed in bracket in such way: {"val1", "val2", "val3"}
     * @param s: tested value
     * @param il: bracket collection
     */
    for (auto v: il) {
        if (s == v) {
            return true;
        }
    }
    return false;
}


bool one_of(string s, vector<string> il) {
    /**
     * Tells if value s exists in vector<string> passed in vector<string>
     * @param s: tested value
     * @param il: vector<string> object
     */
    for (auto v: il) {
        if (s == v) {
            return true;
        }
    }
    return false;
}


void askForSieveLimit(string s, uint *n, uint default=25) {
    /**
     * Depending whether passed string s is empty, it asks user to write a number that is limit of sieve
     * @param s when not empty function tries to convert it to uint. In case of exception it converts it
     * to default value
     * @param *n is a pointer to variable responsible for limit of MMSieve
     */
    (*n) = default;
    uint val = 0;
    string sval = "";
    
    if (s == "") {
        cout << "Write MMSieve limit (number <25; 2000000000>)  Number: ";
        cin >> sval;
    }
    
    try{ 
        val = stoul(sval, nullptr, 0);
        if (val < default || val > 2000000000u) {
            val = default;
        }
    } catch (invalid_argument& ia) {
        val = default;
    }
    (*n) = val;
}


/*void askForThreads(string s, bool *use_threads, uint *active_threads, uint *cpu_cores) {
    if (s != "") {
        if (one_of(s, {"n", "no", "false"})) {
            (*use_threads) = false;
        } else {
            if (one_of(s, {"y", "yes", "true"})) {
                (*use_threads) = true;
                (*active_threads) = (*cpu_cores);
            }
        }
    } else {
        cout << "Use threads? [y]es/[other] for no. Choice: ";
        string choice;
        cin >> choice;
        if (choice == "y") {
            (*use_threads) = true;
            stringstream ss;
            ss << (*cpu_cores);
            cout << "How much threads to use? Value must be in range <" << 0 << ";" << ss.str() << ">  Choice: ";
            uint userThreads = (*cpu_cores);
            try{
                cin >> userThreads;
                if (userThreads >=0 && userThreads < (*cpu_cores)) {
                    (*active_threads) = userThreads;
                } else {
                    (*active_threads) = (*cpu_cores);
                }
            } catch(invalid_argument& ia) {
                (*active_threads) = (*cpu_cores);
            }
        }
    }
}*/


void askForThreadsCount(string s, bool *use_threads, uint *active_threads, uint *cpu_cores) {
    /**
     * Depending on emptiness of @param s can be a console wizard menu for enabling/disabling
     * multithreading. In case of correct answers it allows to decide how many threads should
     * be used, of course if user wants multithreading.
     * 
     * @param s should contain:
     *    - empty value for wizard option
     *    - one value of {"no", "1", "false", "n"}  for disabling multithreading
     *    - numeric value in range <2; cpu cores>  for enabling multithreading
     * 
     * @param *use_threads - pointer to settings in main function. When set to false disables
     *      multithreading
     * @param *active_threads - pointer to settings in main function.  Tells how much threads
     *      should be used in a third stage of MMSieve
     * @param *cpu_cores - pointer to settings in main function.  It tells how much cores has
     *      current machine
     */
    uint val = 1;
    
    if (s == "") {
        string answer;
        cout << "Use threads? [y]es/[other for not]. Choice: ";
        cin >> answer;
        if (answer == "y") {
            (*use_threads) = true;
                    
            cout << "How much threads use? <0; " << *cpu_cores << ">  Choice: ";
            cin >> answer;
            
            try{ 
                val = stoul(s, nullptr, 0);
                if (val < 0 || val > (*cpu_cores)) {
                    val = (*cpu_cores);
                }
            } catch (invalid_argument& ia) {
                val = (*cpu_cores);
            }
            (*active_threads) = val;
            (*use_threads) = true;
            return;
        } else {
            (*use_threads) = false;
            return;
        }
    }
    
    if (s != "") {
        if (one_of(s, {"no", "1", "false", "n"})) {
            (*use_threads) = false;
            (*active_threads) = 1;
            return;
        } else {
            try{ 
                val = stoul(s, nullptr, 0);
                if (val <= 0 || val > (*cpu_cores)) {
                    val = (*cpu_cores);
                    (*use_threads) = true;
                    (*active_threads) = val;
                    return;
                }
                return;
            } catch (invalid_argument& ia) {
                val = (*cpu_cores);
                (*use_threads) = true;
                (*active_threads) = val;
            }
        }
    }
}


void askForFileName(string s, bool *save_to_file, string *fileName) {
    /**
     * Depending on emptiness of @param s it can disable possibility of storing generated primes on disk
     * 
     * @param s - passing one value of {"0", "false", "no"} disables saving to file
     * @paran *save_to_file -  pointer to settings in main function,  tells if saving to file is enabled
     * @param *fileName - pointer to settings in main function, tells what filename should take new file
     */ 
    if (s != "") {
        if (one_of(s, vector<string>{"0", "false", "no"})) {
            (*save_to_file) = false;
        } else {
            (*fileName) = s;
        }
    } else {
        cout << "Write name of a file with generated primes: ";
        cin >> (*fileName);
        if ((*fileName) == "") {
            (*fileName) = "primes.txt";
        }
    }
}


uint nextPrime(uint p, bool s[]) {
    /**
     * Detects first prime number on the right side on number line
     * 
     * @param p - base number against which function looks for next prime
     * @param s[] - array of bools, true symbols primes, false composites 
     * @return numeric position
     */
    uint i = p;
    do {
        i++;
    }
    while (!s[i]);
    return i;
}


void generator(uint p, uint e, uint n, bool s[]) {
    /**
     * Calculates copy-value-distance, the range params of loop, in the loop  copies values from cells away of distance.
     * Copying values from distance cells generates possible prime candidates for further cleaning. Copying false values
     * gives composite numbers, so two goals are realized by one function.
     * 
     * The invention of MMSieve cycles, calculate certain ranges, copy-distance-away values on these ranges, tricky fast
     * cleaning (look next function) makes this sieve the fastest in the world by now (21.03.2022). Little of parameters
     * are passed to short functions, and main storage of primes/composites sits in simple bool array. Polish invention!
     * 
     * @param p - recently generated prime number
     * @param e - precalculated beginning of the MMSieve cycle
     * @param n - MMSieve limit
     * @param s - array with bools, represents prime number for true and false for composites 
     */
    uint distance = e - p + 1;  // copy-value-distance
    uint start = e + 1; // new range starts
    uint end = p * distance + nextPrime(p, s) - 1;  // new range upper limit
    
    if (end >= n || end < 0) { // we do not want to land out of bounds
        end = n;
    }
    for (uint i = start; i <= end; i++) {
        s[i] = s[i - distance]; // select future candidates for primes, ok, invite composites if you must
    }
}


void cleaning(uint p, uint e, bool s[]) {
    /**
     * Select new composites based on prime, use precalculated limit. Other primes also generate composites so therefore
     * we do not have to work on whole range (which may  be large), for each prime, while each iteration. MMSieve cycles 
     * sagaciously selects numbers to be traten as a composites.
     * 
     * To be honest the main case of MMSieve is to find composites...
     * 
     * @param p - previously calculated prime number
     * @param e - previously calculated upper range of MMSieve cycle. 
     * @param s - primes candidates and composited
     * 
     * Decreasing loop produces new composites
     */
    
    //cout << "cleaning p=" << p << " e=" << e << "\n";
    for (uint i = e; i >= p; i--) {
        if (s[i]) {
            s[i * p] = false;
        }
    }
}


void toFile(bool s[], uint len, string name="primes.txt") {
    /**
     * Saves primes in human readable manner to text file
     * 
     * @param s - array of bools
     * @param len - length of s
     * @param name - file name
     * @return 
     */
    ofstream record(name);
    cout << "\nSaving to file " << name << "\n";
    for (uint i=0, k=0; i<= len; i++) {
        if(s[i]) {
            k+=1;
            record << i <<" ";
            if (k%25==0) {
                record << "\n";
            }
        }
    }
    cout << "File saved successfully\n";
    record.close();
}


int main(int argc, char *argv[]) {
    
    bool use_threads = false;
    uint e=2, p=2, n=10000, active_threads=1, cpu_cores=thread::hardware_concurrency();
    string fileName = "primes.txt";
    bool save_to_file = true;
    
    string errMsg = "ERROR: param threads must be number in <0; cores count>.  by default is: ";
    stringstream ss;
    ss << active_threads;
    errMsg += ss.str();
    
    vector<string> args;  // for storing C-argv like a pro in convenient stl::vector
    vector<pair<string, string>> userParams; // parameters split by = and stored as .first and .second
    
    //all arguments to lower
    for (int i=1;  i<argc;  ++i) {
        string arg = string(argv[i]);
        transform(arg.begin(), arg.end(), arg.begin(),
            [](unsigned char c){ return tolower(c); }
        );
        args.push_back(arg);
    }
    
    //all arguments split by = 
    for (auto s: args) {
        if (s.find("=") != string::npos) {
            auto start = 0U;
            auto end = s.find("=");
            string paramName, paramValue;
            while (end != string::npos) {
                paramName = s.substr(start, end - start);
                start = end + 1;
                end = s.find("=", start);
            }
            paramValue = s.substr(start, end);
            userParams.push_back(make_pair(paramName, paramValue));
        }
    }
    
    vector<string> userKeys; // keys already passed to cmdline

    for (auto nextPair: userParams) {
        if (one_of(nextPair.first, {"n", "max", "limit"})) {
            askForSieveLimit(nextPair.second, &n);
            userKeys.push_back("n");
        }
        
        if (one_of(nextPair.first, {"threads", "threads_count", "t"})) {
            askForThreadsCount(nextPair.second, &use_threads, &active_threads, &cpu_cores);
            userKeys.push_back("threads");
        }
        
        if (one_of(nextPair.first, {"f", "file", "output", "filename"})) {
            askForFileName(nextPair.second, &save_to_file, &fileName);
            userKeys.push_back("filename");
        }
        //cout << "cmdline: " << nextPair.first << " = " << nextPair.second << "\n";
    }
    
    /* check what settings were not set yet and launch Gandalf wizard in the case of emergency */
    if (!one_of("n", userKeys)) {
        askForSieveLimit("", &n); 
    }
    if (!one_of("threads", userKeys)) {
        askForThreadsCount("", &use_threads, &active_threads, &cpu_cores);
    }
    if (!one_of("filename", userKeys)) {
        askForFileName("", &save_to_file, &fileName);
        cout << "stf" << save_to_file << "\tname\t" << fileName << "\n\n";
    }
    
    clock_t start = clock(); // HERE WE GO !!!!!!11
    bool *s = new bool[max({n, 6u})];
    s[2] = s[3] = s[5] = true; // predefined primes and remember we had set uint e=2, p=2, n=10000 or more
    
                                                                   /*
     _ _     _    _           _     _                          
  __| (_)_ _(_)__| |___   ___| |_  (_)_ __  _ __  ___ _ _ __ _ 
 / _` | \ V / / _` / -_) / -_)  _| | | '  \| '_ \/ -_) '_/ _` |
 \__,_|_|\_/|_\__,_\___| \___|\__| |_|_|_|_| .__/\___|_| \__,_|
                                           |_|
                                                

 */
    
    cout << "STAGE 1/3\n";
    uint np = 0;
    do {
        generator(p, e, n, s); // lets calc new cycles, copy distance values, produce prime candidates and 
        cleaning(p, e, s); // determine composites in strictly selected surrounding. It will empower magic
        np = nextPrime(p, s); // store found prime in variable instead of invoking function twice, faster!
        e = p * (e - p + 1) + np - 1; // contrive next MMSieve cycle beginning, upper range will be later 
        p = np; // estimated in generator function. To p assign current next_prime result; is it last iter
    }
    while ((p * e <= n && p * e > 0)); // condition of abandon stage1. One day
    cout << "STAGE 2/3\n";
    generator(p, e, n, s); // This generator iteration is the longest. Gamma Goblins works for acceleration
    
    cout << "STAGE 3/3\n";
    
    if (!use_threads) {
        do { // this short, tiny but powerful loop eliminates more and more rare composites still fighting
            cleaning(p, n/p, s); // for pretending to look like prime. Each iteration in against to stage1
            p = nextPrime(p, s); // takes lower set of compsite numbers to be signed as an useless [vulgar]
        } // with other words and letters: each iteration take smaller chunk with less range to evaporize..
        while (p * p <= n); // prime number curse
    } else {
        /*
         * How above simple several lines got totaly mad when they heared: threades?
         * 
         * Got used to lazy meditations with Python Zen interpretations, after several years without C++1z
         * I found infinietly emerging minor but bothersome snippies.To work with threads ,we have to split
         * our work into chunks and decide how to conquer them all, not saving any composities nor their..
         * kids. Understanding why prime numbers are not pleasantly hosted in iterative environment betrays
         * discretely some mysterious about their recursive nature.
         + 
         * The idea is very simple.  I want to use threads to smash looped cleaning and determinations next
         * primes into tasks that willingly acts separately from each other. Tell them to do so, 'll listen
         | 
         * Firstly I notice that:   while ((p * e <= n && p * e > 0));   is the  necessary condition.
         * 
         * So I understand that there is a need to select most important variables and ranges that in near 
         * future will be passed to functions that should work simultanously under the threads. Time comes
         * for preparing structures of data and stuff them fully (charge up to the cap!) with correct data.         
        */
        
        vector<vector<int>> primes; // We would love to use threades, each of them will work separate ...
        
        for (uint i=0;  i<active_threads;  ++i) { // fill each sub-vector with primes, we want to get even
            primes.push_back(vector<int>()); // or give me odds that try to cheat us "we are primes", lol
        }

        for(int i=0; p * p < n; ++i) { // for each future thread I prepare distinct primes for cleaning fun
            primes[i % active_threads].push_back(p); //,,,pushing them to the end
            p = nextPrime(p, s); // slowly drifting to destination
        } 

        for (auto v: primes) { // it serves me some info, something like statistics for nerds on YouTube <3
            cout << "vec size:" << v.size() << "\n";
        }
        
        
        /*
         * Threading in C++17 allows for creating  custom classes with  overloaded operator() returning void
         * 
         */
        class ThreadedTask {
            vector<int> *primes_vec;
            uint limit, thread_id;
            bool *s;
            uint *actives;
        
        public:
        
            ThreadedTask(vector<int> *primes_vec, uint limit, uint thread_id, bool s[], uint *actives) {
                /**
                 * Constructor that earns informations for future job
                 * 
                 * @param *primes_vec is a pointer to previously rendered primes up to sqrt(limit)
                 * @param limit tells how big is @param s (bool array)
                 * @param thread_id has one purpose: statistics
                 * @param s stores prime candidates and composites
                 * @param actives - when our threaded task we inform the world that next thread finished 
                 *      its job
                 */ 
                this->primes_vec = primes_vec;
                this->limit = limit;
                this->thread_id = thread_id;
                this->s = s;
                this->actives = actives;
            }
            
            ~ThreadedTask() {
                
            }
        
            void operator() () {
                /**
                 * This method is automaticaly launched by OS. It is our threaded task. Having passed
                 * pointer to previously created vector with primes we use it for clean and tidy room
                 * with nice primes, elimintaing parasites that tediously tries conterfeit our primes
                 */
                cout << "cleaning thread, thread_id: " << this->thread_id << "\n";
                for (int p: *this->primes_vec) {
                    auto e = round(this->limit / p) + 1;
                    if (e * e < this->limit) {
                        cleaning(p, round(this->limit / p) + 1, s);
                    }
                }
                cout << "cleaning thread " << this->thread_id << "done\n";
                *(this->actives) -= 1;
            }
        }; // end of class
           
        uint threads_to_use = uint(active_threads); // copy value
        
        cout << "ACTIVE THREADS:" << active_threads << "\n\n";
        
        for (uint i=0;  i<threads_to_use;  ++i) { // we launch a threads
            auto t = thread(ThreadedTask(&primes[i], n, i, s, &active_threads));

            if (t.joinable()) {
                t.join();
            }
        }
        
        while (1) { // here we can wait till all threads finish their cleaning jobs
            cout << "at=" << active_threads << "\t";
            //cout << "\twaiting....\n";
            //sleep_until(system_clock::now() + seconds(0.25));
            sleep_until(system_clock::now() + 0.1s);
            if (active_threads == 0) {
                break;
            }
        }
    } 
    
    cout << "Action time " << (float)(clock() - start)/1000 << " ms";
    
    if (save_to_file) {
        toFile(s, n, fileName);
    }
    
    delete []s;
    return 0; 
} // I really do not know if more I hate C++ or more I love it even over Python <3 
