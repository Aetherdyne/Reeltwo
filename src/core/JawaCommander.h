#ifndef JawaCommander_h
#define JawaCommander_h

#include "JawaEvent.h"

/**
  * \ingroup Core
  *
  * \class JawaCommander
  *
  * \brief JAWA Lite command parser
*/
template<int bufferSize=20, int msgSize=20>
class JawaCommander
{
public:
	/**
	  * Parse the specified command string. The specified string is not copied but will not be modified by the parser.
	  * Input is expected to have been sanitized and should contain no CR nor LF.
	  */
	bool parseCommand(const char* inputStr)
	{
		char ch, cmd;
		int addr = 0;
		const char* str = inputStr;
		if ((ch = *str++) == '\0')
			return true;
		if (isdigit(ch))
		{
			do
			{
				addr = addr * 10 + (ch - '0');
				ch = *str++;
			} while (isdigit(ch));
		}
		switch (cmd = ch)
		{
			case 'A':
			case 'D':
				JawaEvent::process(addr, cmd);
				return true;
			case 'B': case 'C':
			case 'E': case 'I':
			case 'N': case 'O':
			case 'R': case 'S':
			case 'T': case 'W':
			{
				int arg = 0;
				ch = *str++;
				if (isdigit(ch))
				{
					do
					{
						arg = arg * 10 + (ch - '0');
						ch = *str++;
					} while (isdigit(ch));
				}
				else
				{
					/* Invalid */
					return false;
				}
				JawaEvent::process(addr, cmd, arg);
				return true;
			}
			case 'L':
				/* Loadable font not supported */
				return false;
			case 'M':
			{
				char *bm = fMessageBuffer;
				char *bme = fMessageBuffer + sizeof(fMessageBuffer) - 1;
				*bm = '\0';
				while ((ch = *str++) != '\0' && bm < bme)
				{
					*bm++ = ch;
					*bm = '\0';
				}
				JawaEvent::process(addr, cmd, fMessageBuffer);
				return true;
			}
			case 'P':
			{
				int arg = 0;
				ch = *str++;
				if (isdigit(ch))
					arg = ch - '0';
				else if (ch >= 'A' && ch <= 'Z')
					arg = 10 + ch - 'A';
				else
					/* Invalid */
					return false;
				ch = *str++;
				if (!isdigit(ch))
					/* Invalid */
					return false;
				int value = 0;
				do
				{
					value = value * 10 + (ch - '0');
					ch = *str++;
				} while (isdigit(ch));
				JawaEvent::process(addr, cmd, arg, value);
				return true;
			}
			case 'V':
			{
				char fname[13];
				char *fn = fname;
				char *fne = fname + sizeof(fname) - 1;
				if ((ch = *str) == '0')
				{
					JawaEvent::process(addr, cmd);
					return true;
				}
				*fn = '\0';
				while ((ch = *str++) != '\0' && fn < fne)
				{
					*fn++ = ch;
					*fn = '\0';
				}
				JawaEvent::process(addr, cmd, fMessageBuffer);
				return true;
			}
			case 'X':
				// TODO
				/* Exmine new channel values not supported */
				return false;
			case 'Z':
				JawaEvent::process(addr, cmd);
				return true;
			case ':':
				/* Ignore comments */
				return false;
			default:
				/* Invalid */
				return false;
		}
		return false;
	}

	/**
	  * Append a single character to the parsers input buffer.
	  */
	void process(const char ch)
	{
		if (fSkipEOL)
		{
			if (ch != '\n' && ch != '\r')
				return;
			fSkipEOL = false;
			fNewLine = true;
		}
		if (fNewLine && isspace(ch))
			return;
		fNewLine = false;
		if (ch == '\n' || ch == '\r')
		{
			if (fPtr != fBuffer)
			{
				parseCommand(fBuffer);
			}
			fPtr = fBuffer;
			fNewLine = true;
		}
		else if (fPtr < fBuffer + sizeof(fBuffer) - 1)
		{
			*fPtr++ = ch;
			*fPtr = '\0';
		}
		else
		{
			parseCommand(fBuffer);
			fPtr = fBuffer;
			fSkipEOL = true;
		}
	}

	/**
	  * Append a string of character to the parsers input buffer.
	  */
	void process(const char* msg)
	{
		for (char ch; *msg++ != '\0';)
			process(ch);
		process('\r');
	}

	/**
	  * Return a direct pointer to the parsers input buffer.
	  */
	inline char* getBuffer()
	{
		return fBuffer;
	}

	/**
	  * Return the maximum size of the input buffer
	  */
	inline size_t getBufferSize()
	{
		return bufferSize;
	}

private:
	bool fNewLine = true;
	bool fSkipEOL;
	char fBuffer[bufferSize];
	char* fPtr = fBuffer;
	char fMessageBuffer[msgSize];
};

#endif
