#include <Storages/Kafka/ReadBufferFromKafkaConsumer.h>

namespace DB
{
namespace
{
    const auto READ_POLL_MS = 500; /// How long to wait for a batch of messages.
} // namespace

void ReadBufferFromKafkaConsumer::commit()
{
    if (messages.empty() || current == messages.begin())
        return;

    auto & previous = *std::prev(current);
    LOG_TRACE(log, "Committing message with offset " << previous.get_offset());
    consumer->async_commit(previous);
}

/// Do commit messages implicitly after we processed the previous batch.
bool ReadBufferFromKafkaConsumer::nextImpl()
{
    if (current == messages.end())
    {
        commit();
        messages = consumer->poll_batch(batch_size, std::chrono::milliseconds(READ_POLL_MS));
        current = messages.begin();

        LOG_TRACE(log, "Polled batch of " << messages.size() << " messages");
    }

    if (messages.empty() || current == messages.end())
        return false;

    if (auto err = current->get_error())
    {
        ++current;

        // TODO: should throw exception instead
        LOG_ERROR(log, "Consumer error: " << err);
        return false;
    }

    // XXX: very fishy place with const casting.
    auto new_position = reinterpret_cast<char *>(const_cast<unsigned char *>(current->get_payload().get_data()));
    BufferBase::set(new_position, current->get_payload().get_size(), 0);

    ++current;

    return true;
}

} // namespace DB
