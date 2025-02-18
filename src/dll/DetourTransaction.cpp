#include "DetourTransaction.hpp"

#include "Utils.hpp"

DetourTransaction::DetourTransaction(const std::source_location aSource)
    : m_source(aSource)
    , m_state(State::Invalid)
{
    spdlog::trace("Trying to start a detour transaction in '{}' ({}:{})", m_source.function_name(),
                  m_source.file_name(), m_source.line());

    auto result = DetourTransactionBegin();
    if (result == NO_ERROR)
    {
        spdlog::trace("Transaction was started successfully", m_source.function_name(), m_source.file_name(),
                      m_source.line());

        QueueThreadsForUpdate();
        SetState(State::Started);
    }
    else
    {
        spdlog::error("Could not start the detour transaction in '{}' ({}:{}). Detour error code: {}",
                      m_source.function_name(), m_source.file_name(), m_source.line(), result);
    }
}

DetourTransaction::~DetourTransaction()
{
    // Abort if the transaction is dangling.
    if (m_state == State::Started)
    {
        Abort();
    }
}

const bool DetourTransaction::IsValid() const
{
    return m_state != State::Invalid;
}

bool DetourTransaction::Commit()
{
    spdlog::trace("Committing the transaction...");

    if (m_state != State::Started && m_state != State::Failed)
    {
        switch (m_state)
        {
        case State::Invalid:
        {
            spdlog::warn("The transaction is in an invalid state");
            break;
        }
        case State::Committed:
        {
            spdlog::warn("The transaction is already committed");
            break;
        }
        case State::Aborted:
        {
            spdlog::warn("The transaction is aborted, can not commit it");
            break;
        }
        default:
        {
            spdlog::warn("Unknown transaction state. State: {}", static_cast<int32_t>(m_state));
            break;
        }
        }

        return false;
    }

    auto result = DetourTransactionCommit();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not commit the transaction. Detours error code: {}", result);
        return false;
    }

    SetState(State::Committed);
    spdlog::trace("The transaction was committed successfully");

    return true;
}

bool DetourTransaction::Abort()
{
    spdlog::trace("Aborting the transaction...");

    if (m_state != State::Started && m_state != State::Failed)
    {
        switch (m_state)
        {
        case State::Invalid:
        {
            spdlog::warn("The transaction is in an invalid state");
            break;
        }
        case State::Committed:
        {
            spdlog::warn("The transaction is committed, can not abort it");
            break;
        }
        case State::Aborted:
        {
            spdlog::warn("The transaction is already aborted");
            break;
        }
        default:
        {
            spdlog::warn("Unknown transaction state. State: {}", static_cast<int32_t>(m_state));
            break;
        }
        }

        return false;
    }

    auto result = DetourTransactionAbort();
    if (result != NO_ERROR)
    {
        // If this happen, we can't abort it.
        SetState(State::Failed);
        spdlog::error("Could not abort the transaction. Detours error code: {}", result);

        return false;
    }

    SetState(State::Aborted);
    spdlog::trace("The transaction was aborted successfully");

    return true;
}

void DetourTransaction::QueueThreadsForUpdate()
{
    spdlog::trace("Queueing threads for detour update...");

    wil::unique_tool_help_snapshot snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0));
    if (!snapshot)
    {
        auto msg = Utils::FormatLastError();
        spdlog::warn(L"Could not create a snapshot of the threads. The transaction will continue but unexpected "
                     L"behavior might happen. Error code: {}, msg: '{}'",
                     GetLastError(), msg);
        return;
    }

    THREADENTRY32 entry;
    entry.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(snapshot.get(), &entry))
    {
        auto msg = Utils::FormatLastError();
        spdlog::warn(L"Could not get the first thread entry from the snapshot. The transaction will continue but "
                     L"unexpected behavior might happen. Error code: {}, msg: '{}'",
                     GetLastError(), msg);
        return;
    }

    auto processId = GetCurrentProcessId();
    auto threadId = GetCurrentThreadId();

    bool shouldContinue = true;
    do
    {
        if (entry.th32OwnerProcessID == processId && entry.th32ThreadID != threadId)
        {
            wil::unique_handle handle(
                OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, false, entry.th32ThreadID));
            if (handle)
            {
                auto result = DetourUpdateThread(handle.get());
                if (result == NO_ERROR)
                {
                    m_handles.emplace_back(std::move(handle));
                }
                else
                {
                    spdlog::warn(L"Could not queue the thread for update. The transaction will continue but unexpected "
                                 L"behavior might happen. Thread ID: {}, handle: {}, detour error code: {}",
                                 entry.th32ThreadID, handle.get(), result);
                }
            }
            else
            {
                auto msg = Utils::FormatLastError();
                spdlog::warn(L"Could not open a thread. The transaction will continue but unexpected behavior might "
                             L"happen. Thread ID: {}, error code: {}, msg: '{}'",
                             entry.th32ThreadID, GetLastError(), msg);
            }
        }

        shouldContinue = Thread32Next(snapshot.get(), &entry);
        if (!shouldContinue && GetLastError() != ERROR_NO_MORE_FILES)
        {
            auto msg = Utils::FormatLastError();
            spdlog::warn(L"Could not get the next thread entry from the snapshot. The transaction will continue but "
                         L"unexpected behavior might happen. Error code: {}, msg: '{}'",
                         GetLastError(), msg);
        }
    } while (shouldContinue);

    spdlog::trace("{} thread(s) queued for detour update (excl. current thread)", m_handles.size());
}

void DetourTransaction::SetState(const State aState)
{
    m_state = aState;
}
