#include <bits/stdc++.h>
using namespace std;

static inline string trim(const string &s){
    size_t i=0,j=s.size();
    while(i<j && isspace((unsigned char)s[i])) ++i;
    while(j>i && isspace((unsigned char)s[j-1])) --j;
    return s.substr(i,j-i);
}

static vector<string> read_program_lines(istream &in){
    vector<string> lines;
    string line;
    while (std::getline(in, line)){
        if (trim(line) == "endprogram") break;
        lines.push_back(line);
    }
    return lines;
}

static vector<string> tokenize(const string &text){
    vector<string> toks;
    auto push_tok = [&](const string &t){ if(!t.empty()) toks.push_back(t); };
    string cur;
    auto flush = [&](){ if(!cur.empty()){ push_tok(cur); cur.clear(); }};
    static const unordered_set<string> keywords = {
        "function","block","set","if","while","for","array.create","array.set","array.get",
        "print","call","return","let","lambda","begin","end","true","false"
    };
    for(size_t i=0;i<text.size();++i){
        char c=text[i];
        if (isspace((unsigned char)c)) { flush(); continue; }
        if (c=='(' || c==')' || c=='[' || c==']' || c=='{' || c=='}') {
            flush(); string s(1,c); push_tok(s); continue;
        }
        if (ispunct((unsigned char)c)){
            // keep dot as part of word to preserve array.set
            if (c=='.') { cur.push_back(c); continue; }
            flush(); string s(1,c); push_tok(s); continue;
        }
        if (isdigit((unsigned char)c)){
            // consume full number
            string num; size_t j=i; bool hasdot=false;
            while(j<text.size() && (isdigit((unsigned char)text[j]) || (!hasdot && text[j]=='.'))){
                if(text[j]=='.') hasdot=true; num.push_back(text[j]); ++j;
            }
            i=j-1; flush(); push_tok("NUM"); continue;
        }
        if (isalpha((unsigned char)c) || c=='_' ){
            string id; size_t j=i;
            while(j<text.size() && (isalnum((unsigned char)text[j]) || text[j]=='_' || text[j]=='.')){
                id.push_back(text[j]); ++j;
            }
            i=j-1; flush();
            string low=id; for(char &ch:low) ch=tolower((unsigned char)ch);
            if (keywords.count(low)) push_tok(low);
            else push_tok("ID");
            continue;
        }
        // default: treat as single char token
        flush(); string s(1,c); push_tok(s);
    }
    flush();
    return toks;
}

static unordered_set<uint64_t> shingles(const vector<string>& toks, int k){
    unordered_set<uint64_t> S; if ((int)toks.size()<k) return S;
    const uint64_t B = 1315423911u;
    for (int i=0;i+(k-1)<(int)toks.size();++i){
        uint64_t h=1469598103934665603ull; // FNV-like mix
        for (int j=0;j<k;++j){
            const string &t = toks[i+j];
            uint64_t x=0; for(unsigned char c: t){ x = x*B + c + 1; }
            h ^= x; h *= 1099511628211ull;
        }
        S.insert(h);
    }
    return S;
}

static double jaccard(const unordered_set<uint64_t>& A, const unordered_set<uint64_t>& B){
    if (A.empty() && B.empty()) return 0.5; // unknown
    size_t inter=0, uni=A.size()+B.size();
    if (A.size() < B.size()){
        for (auto &x: A) if (B.count(x)) ++inter;
    } else {
        for (auto &x: B) if (A.count(x)) ++inter;
    }
    uni -= inter;
    if (uni==0) return 1.0;
    return (double)inter / (double)uni;
}

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);

    // Read entire stdin into lines to support both modes (cheat and anticheat)
    vector<string> all;
    string line;
    while (std::getline(cin, line)) all.push_back(line);

    // Find lines equal to "endprogram" (trimmed)
    vector<size_t> ends;
    for (size_t i=0;i<all.size();++i){ if (trim(all[i])=="endprogram") ends.push_back(i); }

    if (ends.size() >= 2){
        // Anticheat mode: two programs separated by endprogram markers
        vector<string> p1(all.begin(), all.begin()+ends[0]);
        vector<string> p2(all.begin()+ends[0]+1, all.begin()+ends[1]);

        string s1, s2;
        if (!p1.empty()){
            s1.reserve( (size_t)accumulate(p1.begin(), p1.end(), 0ull, [](uint64_t a, const string& b){ return a + b.size() + 1; }) );
            for (auto &l1: p1){ s1 += l1; s1.push_back('\n'); }
        }
        if (!p2.empty()){
            s2.reserve( (size_t)accumulate(p2.begin(), p2.end(), 0ull, [](uint64_t a, const string& b){ return a + b.size() + 1; }) );
            for (auto &l2: p2){ s2 += l2; s2.push_back('\n'); }
        }

        auto t1 = tokenize(s1);
        auto t2 = tokenize(s2);

        // Build shingles with multiple k and average for robustness
        vector<int> ks = {2,3,4};
        double sum=0.0; int cnt=0;
        for(int k: ks){
            auto sA = shingles(t1,k);
            auto sB = shingles(t2,k);
            sum += jaccard(sA, sB);
            ++cnt;
        }
        double s = cnt? (sum / cnt) : 0.5;

        // Clamp to [0,1] and output with sufficient precision
        if (s < 0.0) s = 0.0; if (s > 1.0) s = 1.0;
        cout.setf(std::ios::fixed); cout<<setprecision(6)<<s<<"\n";
    } else {
        // Cheat mode fallback: echo input program unchanged to stdout
        for (const auto &l : all) cout << l << '\n';
    }

    return 0;
}
