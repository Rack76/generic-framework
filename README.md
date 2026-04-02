Generic is a framework for game engine and game development.
You can use the entity manager to manage your entities and their associated components.
Register your component types with the component manager before using them, preferably during program initialization.
You can create your own systems by deriving them from the System class.
Once this is done, you can add batches to read and write entities with specified components inside your system, you can also add callbacks to your systems which will react when a specific component type is added or removed.
