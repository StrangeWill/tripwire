#include <node.h>
#include <v8.h>
#include <nan.h>

using namespace v8;

extern NAN_METHOD(resetTripwireCore);
extern void initCore();

unsigned int tripwireThreshold;
Persistent<Value> context;
int terminated;

NAN_METHOD(clearTripwire)
{
	NanScope();

  // Seting tripwireThreshold to 0 indicates to the worker process that
  // there is no threshold to enforce. The worker process will make this determination
  // next time it is signalled, there is no need to force an extra context switch here
  // by explicit signalling.

	tripwireThreshold = 0;
	terminated = 0;
	NanDisposePersistent(context);
  NanReturnValue(NanUndefined());
}

NAN_METHOD(resetTripwire)
{
  NanScope();

	if (0 == args.Length() || !args[0]->IsUint32())
		return NanThrowError("First agument must be an integer time threshold in milliseconds.");

	if (0 == args[0]->ToUint32()->Value())
		return NanThrowError("The time threshold for blocking operations must be greater than 0.");

	clearTripwire(args);
	tripwireThreshold = args[0]->ToUint32()->Value();
	if (args.Length() > 1)
	{
    NanAssignPersistent(context, args[1]);
	}

  resetTripwireCore(args);
	NanReturnValue(NanUndefined());
}

NAN_METHOD(getContext)
{
  NanScope();

  // If the script had been terminated by tripwire, returns the context passed to resetTripwire;
  // otherwise undefined. This can be used from within the uncaughtException handler to determine
  // whether the exception condition was caused by script termination.
  if (terminated)
  	NanReturnValue(context);
  else
  	NanReturnValue(NanUndefined());
}

void init(Handle<Object> target)
{
  initCore();
  terminated = 0;
  NODE_SET_METHOD(target, "resetTripwire", resetTripwire);
  NODE_SET_METHOD(target, "clearTripwire", clearTripwire);
  NODE_SET_METHOD(target, "getContext", getContext);
}

NODE_MODULE(tripwire, init);
