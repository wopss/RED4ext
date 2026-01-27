#include "DetourTransaction.hpp"
#include "Utils.hpp"

#include <Windows.h>
#include <winternl.h>

#include <detours.h>
#include <spdlog/spdlog.h>
#include <wil/resource.h>

#include <cstdint>
#include <source_location>
#include <utility>

extern "C" NTSYSCALLAPI NTSTATUS NTAPI NtGetNextThread(_In_ HANDLE ProcessHandle, _In_opt_ HANDLE ThreadHandle,
                                                       _In_ ACCESS_MASK DesiredAccess, _In_ ULONG HandleAttributes,
                                                       _In_opt_ _Reserved_ ULONG Flags, _Out_ PHANDLE NewThreadHandle);

DetourTransaction::DetourTransaction(const std::source_location aSource)
    : m_source(aSource)
    , m_state(State::Invalid)
    , m_hasHeapLock(false)
{
    spdlog::trace("Trying to start a detour transaction in '{}' ({}:{})", m_source.function_name(),
                  m_source.file_name(), m_source.line());

    auto hasLock = HeapLock(GetProcessHeap());
    if (!hasLock)
    {
        spdlog::error("Could not lock the process heap in '{}' ({}:{}). Last error: {}", m_source.function_name(),
                      m_source.file_name(), m_source.line(), GetLastError());
        return;
    }

    m_hasHeapLock = true;

    auto result = DetourTransactionBegin();
    if (result == NO_ERROR)
    {
        spdlog::trace("Transaction was started successfully", m_source.function_name(), m_source.file_name(),
                      m_source.line());

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

    if (m_hasHeapLock)
    {
        HeapUnlock(GetProcessHeap());
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

    if (!QueueThreadsForUpdate())
    {
        spdlog::error("Cannot continue with committing the transaction due to failure in queuing threads for update");
        Abort();

        return false;
    }

    auto result = DetourTransactionCommit();
    if (result != NO_ERROR)
    {
        // Detours already aborts the transaction if commit fails.
        SetState(State::Aborted);
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

bool DetourTransaction::QueueThreadsForUpdate()
{
    spdlog::trace("Queueing threads for detour update...");

    const HANDLE currentProcess = GetCurrentProcess();
    const DWORD currentThreadId = GetCurrentThreadId();

    static constexpr ACCESS_MASK threadAccess =
        THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_QUERY_LIMITED_INFORMATION | THREAD_SUSPEND_RESUME;

    HANDLE thread = nullptr;
    bool closePrevThread = false;

    while (true)
    {
        HANDLE nextThread = nullptr;

        NTSTATUS status = NtGetNextThread(currentProcess, thread, threadAccess, 0, 0, &nextThread);
        if (closePrevThread)
        {
            CloseHandle(thread);
        }

        if (!NT_SUCCESS(status))
        {
            break;
        }

        thread = nextThread;
        closePrevThread = true;

        const DWORD threadId = GetThreadId(thread);
        if (threadId == 0)
        {
            auto lastError = GetLastError();
            spdlog::warn("Could not get thread ID. handle: {}, lastError: {}", thread, lastError);

            continue;
        }

        if (threadId == currentThreadId)
        {
            continue;
        }

        // https://ntdoc.m417z.com/threadinfoclass
        static constexpr THREADINFOCLASS ThreadIsTerminated = (THREADINFOCLASS)0x14;

        BOOL isTerminated = FALSE;
        status = NtQueryInformationThread(thread, ThreadIsTerminated, &isTerminated, sizeof(isTerminated), nullptr);
        if (!NT_SUCCESS(status))
        {
            spdlog::warn("Could not query thread information. threadId: {}, handle: {}, status: {}", threadId, thread,
                         status);
            continue;
        }

        if (isTerminated)
        {
            continue;
        }

        const LONG result = DetourUpdateThread(thread);
        if (result == NO_ERROR)
        {
            m_handles.emplace_back(thread);
            closePrevThread = false;
        }
        else
        {
            spdlog::warn("Could not queue the thread for update. threadId: {}, handle: {}, error code: {}", threadId,
                         thread, result);
            return false;
        }
    }

    spdlog::trace("{} thread(s) queued for detour update (excl. current thread)", m_handles.size());
    return true;
}

void DetourTransaction::SetState(const State aState)
{
    m_state = aState;
}
