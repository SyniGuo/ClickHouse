#pragma once

#include <DataStreams/IBlockInputStream.h>
#include <IO/DelimitedReadBuffer.h>
#include <Interpreters/Context.h>
#include <Storages/Kafka/ReadBufferFromKafkaConsumer.h>

namespace DB
{
class StorageKafka;

class KafkaBlockInputStream : public IBlockInputStream
{
public:
    KafkaBlockInputStream(StorageKafka & storage_, const Context & context_, const String & schema, size_t max_block_size_);
    ~KafkaBlockInputStream() override;

    String getName() const override;
    Block readImpl() override;
    Block getHeader() const override;
    void readPrefixImpl() override;
    void readSuffixImpl() override;

private:
    StorageKafka & storage;
    Context context;
    size_t max_block_size;

    ConsumerPtr consumer;
    std::unique_ptr<DelimitedReadBuffer> buffer;
    bool finalized = false;

    // Return true if consumer has been claimed by the stream
    bool hasClaimed() { return consumer != nullptr; }
};

} // namespace DB
