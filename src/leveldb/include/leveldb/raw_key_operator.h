// Copyright (c) 2015, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TERA_LEVELDB_UTILS_RAW_KEY_OPERATOR_H
#define TERA_LEVELDB_UTILS_RAW_KEY_OPERATOR_H

#include <stdint.h>

#include "leveldb/slice.h"

namespace leveldb {

enum TeraKeyType {
    TKT_FORSEEK = 0,
    TKT_TXN_COMMIT,
    TKT_TXN,
    TKT_DEL,
    TKT_DEL_COLUMN,
    TKT_DEL_QUALIFIERS,
    TKT_DEL_QUALIFIER,
    TKT_VALUE,
    TKT_TYPE_NUM,
    TKT_ADD,
    TKT_PUT_IFABSENT,
    TKT_APPEND,
    TKT_ADDINT64,
};

class RawKeyOperator {
public:
    virtual void EncodeTeraKey(const std::string& row_key,
                               const std::string& family,
                               const std::string& qualifier,
                               int64_t timestamp,
                               TeraKeyType type,
                               std::string* tera_key) const = 0;

    virtual bool ExtractTeraKey(const Slice& tera_key,
                                Slice* row_key,
                                Slice* family,
                                Slice* qualifier,
                                int64_t* timestamp,
                                TeraKeyType* type) const = 0;
    virtual int Compare(const Slice& key1,
                        const Slice& key2) const = 0;

    void EncodeTeraValue(const std::string& value, int64_t txn_id,
                         std::string* tera_value) const {
        if (txn_id == 0) {
            tera_value->assign(value);
            return;
        }
        tera_value->assign("txn");
        char txn_id_str[8];
        memcpy(txn_id_str, &txn_id, 8);
        tera_value->append(std::string(txn_id_str, 8));
        tera_value->append(value);
    }

    void ExtractTeraValue(const Slice& tera_value, int64_t* txn_id, Slice* value) const {
        if (tera_value.size() <= 3 + 8 || strncmp(tera_value.data(), "txn", 3) != 0) {
            *value = tera_value;
            return;
        }
        memcpy((char*)txn_id, tera_value.data() + 3, 8);
        Slice payload(tera_value.data() + 11, tera_value.size() - 11);
        *value = payload;
    }
};

const RawKeyOperator* ReadableRawKeyOperator();
const RawKeyOperator* BinaryRawKeyOperator();
const RawKeyOperator* KvRawKeyOperator();

} // namespace leveldb
#endif //TERA_LEVELDB_UTILS_RAW_KEY_OPERATOR_H
