/** Description:
This C++ header defines a foundational blueprint, or interface, named `Serializable`. This interface standardizes how complex objects manage the saving and loading of their internal data.

**The Core Purpose**

The primary goal is to ensure that any object built upon this foundation can be reliably converted into a generic, simple list of properties suitable for external storage (like writing to a file, sending over a network, or saving to a database). It also provides the necessary mechanism to perfectly reconstruct the object from that saved list. This entire process is known as serialization.

**Key Data Structures**

The system standardizes the types of data that can be handled. A generic data field, called `Value`, is defined to strictly hold either plain text or a single decimal number. All data is organized into simple **Key-Value pairs**—an identifying name (the key) is matched with its specific content (the value). This ensures portability regardless of the object's complexity.

**Required Behaviors (The Contract)**

Any object that uses this blueprint must implement two crucial actions:

1.  **Serialization (Saving):** The object must generate a complete list of its internal Key-Value pairs so its current state can be captured and saved externally.
2.  **Hydration (Loading):** The object must be able to take a list of Key-Value pairs and use that information to load or reset its internal properties, effectively restoring its state from the saved data.

This setup enforces consistency across different parts of a software system, guaranteeing that complex data can always be safely transferred, stored, and retrieved.

sha: 12ac5ba753299b341d4a8834a79c596e43ebf8fb40a317ae3437933d30041d58 
*/
#pragma once

#include <string>
#include <variant>
#include <vector>

class Serializable {
public:
    using Value = std::variant<std::string, float>;
    struct KeyValue {
        std::string key;
        Value value;
    };

    virtual void hydrate(const std::vector<KeyValue>& values) = 0;
    virtual std::vector<KeyValue> serialize() const = 0;
};
