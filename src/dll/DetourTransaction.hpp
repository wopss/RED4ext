#pragma once

class DetourTransaction
{
public:
    DetourTransaction(std::source_location aSource = std::source_location::current());
    ~DetourTransaction();

    DetourTransaction(DetourTransaction&) = delete;
    DetourTransaction(DetourTransaction&&) = delete;

    DetourTransaction& operator=(const DetourTransaction&) = delete;
    DetourTransaction& operator=(DetourTransaction&&) = delete;

    const bool IsValid() const;

    bool Commit();
    bool Abort();

private:
    enum class State : uint8_t
    {
        Invalid,
        Started,
        Committed,
        Aborted,
        Failed
    };

    void SetState(State aState);
    void QueueThreadsForUpdate();

    const std::source_location m_source;
    State m_state;
    std::vector<wil::unique_handle> m_handles;
};
