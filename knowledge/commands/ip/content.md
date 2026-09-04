# ip - inspect network interfaces and routes

The `ip` command from iproute2 displays and configures Linux networking.

```bash
ip address show
ip link show
ip route show
ip -brief address show
```

`address` lists protocol addresses, `link` shows network interfaces, and
`route` displays the routing table. The abbreviated `-brief` output is useful
for a compact overview. These forms are read-only and normally need no special
privileges.

Changing addresses, links, or routes usually requires administrator
privileges and can immediately disconnect local or remote sessions. Record the
current configuration and understand how the system's network manager will
persist or replace a change before using `add`, `delete`, or `set` operations.
