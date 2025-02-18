#include "DevConsole.hpp"

#include "Utils.hpp"

DevConsole::DevConsole(const Config::DevConfig& aConfig)
    : m_isCreated(false)
    , m_stdoutStream(nullptr)
    , m_stderrStream(nullptr)
{
    if (aConfig.hasConsole)
    {
        if (AllocConsole())
        {
            m_isCreated = true;

            SetConsoleTitle(L"RED4ext Console");

            // Disable the close button / context menu.
            auto console = GetConsoleWindow();
            if (console)
            {
                auto menu = GetSystemMenu(console, false);
                if (menu)
                {
                    DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
                }
            }

            // Now redirect the output.
            auto err = freopen_s(&m_stdoutStream, "CONOUT$", "w", stdout);
            if (err != 0)
            {
                SHOW_MESSAGE_BOX_FILE_LINE(MB_ICONWARNING | MB_OK,
                                           L"Could not redirect the standard output to console.");
            }

            err = freopen_s(&m_stderrStream, "CONOUT$", "w", stderr);
            if (err != 0)
            {
                SHOW_MESSAGE_BOX_FILE_LINE(MB_ICONWARNING | MB_OK,
                                           L"Could not redirect the standard error output to console.");
            }
        }
        else
        {
            SHOW_LAST_ERROR_MESSAGE_FILE_LINE(L"Could not create the development console.");
        }
    }
}

DevConsole::~DevConsole()
{
    if (m_stdoutStream)
    {
        fflush(stdout);
        fclose(m_stdoutStream);

        m_stdoutStream = nullptr;
    }

    if (m_stderrStream)
    {
        fflush(stderr);
        fclose(m_stderrStream);

        m_stderrStream = nullptr;
    }

    if (m_isCreated)
    {
        FreeConsole();
    }
}

bool DevConsole::IsOutputRedirected() const
{
    return m_stdoutStream || m_stderrStream;
}
