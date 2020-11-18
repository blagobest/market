//
//  Parser.hpp
//  Market
//
//  Created by Blagovest on 17/11/20.
//

#ifndef Fix_Parser_hpp
#define Fix_Parser_hpp

#include <string>
#include <map>
#include <vector>
#include <list>

#include "tags/Fix4.2.hpp"
#include "tags/Fix5.0.hpp"

namespace mkt {
namespace fix {

    static const int TAG_MSG_TYPE = 35;
    static const int TAG_CHECKSUM = 10;

    // This could be made better by
    // conditionally using std::distance only if it's O(1)
    // and otherwise using the count(Iterator cur) to count
    // as we iterate over the original string.
    template <typename String>
    class LengthEvaluator {
        using Iterator = typename String::iterator;

        Iterator begin_msg_type, begin_checksum;
        size_t len;
    public:
        void count(Iterator cur, int tag) noexcept {
            // see where we are so we can calculate the length
            if (tag == TAG_MSG_TYPE) {
                begin_msg_type = cur;
            } else if (tag == TAG_CHECKSUM) {
                begin_checksum = cur;
                len = std::distance(begin_msg_type, begin_checksum);
            }
        }
        
        void count(Iterator cur) noexcept {}
        
        size_t length() const noexcept { return len; }
    };
    
    template <typename StoragePolicy, typename String = std::string_view, typename LengthCalculator = LengthEvaluator<String>>
    class Parser {
        const unsigned char SOH = 0x1;
        const unsigned char EQUAL = '=';
        
        unsigned char checksum;
        StoragePolicy storage;
        LengthCalculator length_calc;
        
        void parse(const String & message) {
            auto cur = message.begin();
            unsigned char sum = 0;
            while (cur != message.end()) {
                // Read tag
                auto sep = cur;
                int tag = 0;
                while (sep != message.end() && *sep != EQUAL) {
                    tag = 10 * tag + *sep - '0';
                    sum += *sep;
                    length_calc.count(sep);
                    ++sep;
                }
                length_calc.count(cur, tag);
                // Skip EQUAL
                sum += EQUAL;
                ++sep;
                // Find SOH
                cur = sep;
                while (cur != message.end() && *cur != SOH) {
                    sum += *cur;
                    ++cur;
                }
                storage.store(tag, sep, cur);
                // Skip SOH
                std::advance(cur, 1);
                // Save checksum
                if (tag == TAG_CHECKSUM) {
                    checksum = sum;
                }
            }
        }
    public:
        Parser(const String & message): storage(message) {
            parse(message);
        }
        
        size_t length() const { return length_calc.length(); }
    };

    template <typename String, typename Map = std::map<int, String>>
    class MapBasedStoragePolicy {
        using Iterator = typename String::const_iterator;
        Map map;
    public:
        MapBasedStoragePolicy(const String & message): map() {}
        
        void store(int tag, Iterator start, Iterator end) {
            // std::cerr << tag << " -> " << std::string(start, end) << std::endl;
            map[tag] = String(start, std::distance(start, end)); // FIXME: use C++20 (begin, end) constructor someday
        }
        
        void done() {
            
        }
    };

    template <typename String, typename Sequence = std::vector< std::pair<int, String> >>
    class SequenceBasedStoragePolicy {
        using Iterator = typename String::const_iterator;
        Sequence sequence;
    public:
        SequenceBasedStoragePolicy(const String & message): sequence() {}
        
        void store(int tag, Iterator start, Iterator end) {
            // std::cerr << tag << " -> " << std::string(start, end) << std::endl;
            sequence.emplace_back(tag, String(start, std::distance(start, end))); // FIXME: use C++20 (begin, end) constructor someday
        }
    };

    template <typename String>
    using VectorBasedStoragePolicy = SequenceBasedStoragePolicy<String, std::vector< std::pair<int, String> >>;

    template <typename String>
    using ListBasedStoragePolicy = SequenceBasedStoragePolicy<String, std::list< std::pair<int, String> >>;
}
}

#endif /* Parser_hpp */
