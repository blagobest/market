//
//  Tag.hpp
//  Market
//
//  Created by Blagovest on 18/11/20.
//

#ifndef Fix_Tags_Tag_h
#define Fix_Tags_Tag_h

#include <string>
#include <utility>

namespace mkt {
namespace fix {

    struct Tag {
        enum Originator {
            BUY, SELL, NA
        };
        
        int id;
        std::string name;
        Originator originator;
        
        int min_version;
        int max_version;
        int deprecated_since;
        
        Tag(int id, std::string name, Originator originator, int min_version, int max_version, int deprecated_since): id(id), name(std::move(name)), originator(originator), min_version(min_version), max_version(max_version), deprecated_since(deprecated_since) {}
        
        operator int () const { return id; }
        
        static const Tag & MsgType;
        static const Tag & Checksum;
        static const Tag & BeginString;
    };

}
}

#endif /* Tag_h */
