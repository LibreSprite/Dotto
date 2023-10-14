#pragma once

#include <stdio.h>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <queue>
#include <unistd.h>
#include <fcntl.h>

#include "CircleBuffer.hpp"
#include "Events.hpp"
#include "Index.hpp"
#include "Node.hpp"
#include "VM.hpp"

#define	NLO_RDONLY	0	/* +1 == FREAD */
#define	NLO_WRONLY	1	/* +1 == FWRITE */
#define	NLO_RDWR	2	/* +1 == FREAD|FWRITE */
#define	NLO_APPEND	0x0008	/* append (writes guaranteed at the end) */
#define	NLO_CREAT	0x0200	/* open with file create */
#define	NLO_TRUNC	0x0400	/* open with truncation */
#define	NLO_EXCL	0x0800	/* error on open if file exists */
#define NLO_SYNC	0x2000	/* do all writes synchronously */
#define	NLO_NONBLOCK	0x4000	/* non blocking I/O (POSIX style) */
#define	NLO_NOCTTY	0x8000	/* don't assign a ctty on this open */

class VMImpl : public VM {
public:
    static void reservedAPISlot(const VM::Args&){}

    AutoIndex key{this};
    void* app{};

    class File {
    public:
	FILE* res{};

	File(FILE* res) : res{res} {}
	File(File&& f) : res{f.res} {f.res = nullptr;}

	void close() {
	    if (res) {
		fclose(res);
		res = nullptr;
	    }
	}

	~File() {close();}
    };
    std::vector<File> files;

    template<typename APP>
    VMImpl(APP* app) : app{app} {
        addAPI({
                {"getId", [=](const VM::Args& args) {
		    auto that = static_cast<VMImpl*>(args.vm);
                    args.result = *that->key;
                }},

                {"yield", +[](const VM::Args& args) {args.vm->yield();}},

                {"vmOpen", +[](const VM::Args& args) {
		    auto that = static_cast<VMImpl*>(args.vm);
                    auto fileName = args.get<std::string>(0);
                    auto mode = args.get<uint32_t>(1);
		    mode &= O_RDONLY | O_WRONLY | O_CREAT | O_TRUNC | O_APPEND | O_RDWR;
		    const char* fmode = nullptr;
		    switch (mode) {
		    case NLO_RDONLY                          : fmode = "rb";  break;
		    case NLO_WRONLY | NLO_CREAT | NLO_TRUNC  : fmode = "wb";  break;
		    case NLO_WRONLY | NLO_CREAT | NLO_APPEND : fmode = "ab";  break;
		    case NLO_RDWR                            : fmode = "r+b"; break;
		    case NLO_RDWR | NLO_CREAT | NLO_TRUNC    : fmode = "w+b"; break;
		    case NLO_RDWR | NLO_CREAT | NLO_APPEND   : fmode = "a+b"; break;
		    default:
			LOG("Invalid file mode: ", mode);
			args.result = -1;
			return;
		    }

		    FILE* res = fopen(fileName.c_str(), fmode);
		    if (!res) {
			args.result = -1;
		    } else {
			args.result = static_cast<uint32_t>(that->files.size()) + 3;
			that->files.push_back({res});
		    }
                }},

                {"vmClose", +[](const VM::Args& args) {
		    auto that = static_cast<VMImpl*>(args.vm);
                    auto num = args.get<uint32_t>(0) - 3;
		    if (num >= that->files.size()) {
			return;
		    }
		    that->files[num].close();
		}},

                {"vmLSeek", +[](const VM::Args& args) {
		    auto that = static_cast<VMImpl*>(args.vm);
                    auto fh = args.get(0);
                    auto off = args.get(1);
                    auto whence = args.get(2);
		    args.result = 0;
		    if (fh < 3) {
			return;
		    }
		    fh -= 3;
		    if (fh >= that->files.size() || !that->files[fh].res) {
			return;
		    }
		    if (fseek(that->files[fh].res, off, whence) == 0) {
			args.result = static_cast<int32_t>(ftell(that->files[fh].res));
		    }
		}},

                {"vmWrite", +[](const VM::Args& args) {
		    auto that = static_cast<VMImpl*>(args.vm);
                    auto fh = args.get(0);
                    auto buffer = args.get<char *>(1);
                    auto len = args.get(2);
		    if (fh < 3) {
			args.result = static_cast<uint32_t>(write(fh, buffer, len));
			return;
		    }
		    fh -= 3;
		    if (fh >= that->files.size() || !that->files[fh].res) {
			args.result = 0;
			return;
		    }
		    args.result = static_cast<uint32_t>(fwrite(buffer, 1, len, that->files[fh].res));
                }},

                {"vmRead", +[](const VM::Args& args) {
		    auto that = static_cast<VMImpl*>(args.vm);
                    auto fh = args.get(0);
                    auto buffer = args.get<char *>(1);
                    auto len = args.get(2);
		    if (fh < 3) {
			args.result = static_cast<uint32_t>(read(fh, buffer, len));
			return;
		    }
		    fh -= 3;
		    if (fh >= that->files.size() || !that->files[fh].res) {
			args.result = 0;
			return;
		    }
		    args.result = static_cast<uint32_t>(fread(buffer, 1, len, that->files[fh].res));
                }},

                {"pollEvents", +[](const VM::Args& args) {
		    auto that = static_cast<VMImpl*>(args.vm);
                    std::lock_guard lock{that->ebm};
                    auto ret = that->eventBuffer.empty() ? EventId::MaxEvent : that->eventBuffer.pop();
                    args.result = static_cast<uint32_t>(ret);
                }},

		{"popMessage", +[](const VM::Args& args) {
		    args.result = 0;
		    auto that = static_cast<VMImpl*>(args.vm);
		    that->activeMessage.clear();
		    if (!that->messages.read([](auto& messages){return messages.empty();})) {
			that->messages.write([=](auto& messages){
			    that->activeMessage = std::move(messages.front());
			    messages.pop();
			});
			args.result = static_cast<uint32_t>(that->activeMessage.size());
		    }
		}},

		{"getMessageArg", +[](const VM::Args& args) {
		    auto that = static_cast<VMImpl*>(args.vm);
		    auto index = args.get<uint32_t>(0);
		    if (index < that->activeMessage.size()) {
			args.result = that->activeMessage[index];
		    } else {
			args.result = "";
		    }
		}},

                {"vmExit", +[](const VM::Args& args){
		    auto that = static_cast<VMImpl*>(args.vm);
                    reinterpret_cast<APP*>(that->app)->vmpool.release(*that);
                    that->yield();
                }},

                {"enableEvent", +[](const VM::Args& args) {
		    auto that = static_cast<VMImpl*>(args.vm);
                    auto eventId = args.get(0);
                    if (eventId < static_cast<uint32_t>(EventId::MaxEvent)) {
                        createListenerSlot(static_cast<EventId>(eventId), {
                            .endpoint = that,
                            .handler = +[](void* ptr, EventId id) {
                                reinterpret_cast<VMImpl*>(ptr)->event(id);
                            }
                        });
                    }
                }}
            });
    }

    ~VMImpl() {
        eventListeners.write([&](auto& eventListeners) {
            for (std::size_t id = 0; id < static_cast<uint32_t>(EventId::MaxEvent); ++id) {
                for (auto& slot : eventListeners[id]) {
                    if (slot.endpoint == this)
                        slot = EventListener{};
                }
            }
        });
    }

    void message(std::vector<std::string>&& msg) {
	messages.write([&](auto& messages) {
	    messages.push(std::move(msg));
	});
    }

    void event(EventId id) {
        std::lock_guard lock{ebm};
        eventBuffer.push(id);
    }

private:
    std::mutex ebm;
    CircleBuffer<EventId> eventBuffer{32};
    std::vector<std::string> activeMessage;
    Shared<std::queue<std::vector<std::string>>> messages;
};
