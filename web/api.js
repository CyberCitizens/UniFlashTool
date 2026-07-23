const host = "http://localhost"

// Endpoint is from the host.
// Handles the dirty work of rewriting the same piece of code 40x times because I use fetch APIs
// instead of jquery or; why not, a framework like ReactJS or Angular, maybe I should also
// download the whole .NET framework for a web page that could have weighted 3kb in RAM, who knows ?
export class Query
{
	static host = host;
	static port = "6767";
	
	static async request(method, endpoint, body) {
		const response = await fetch(`${this.host}:${this.port}${endpoint}`,
		{
			method,
			headers: body ? { "Content-Type": "application/json" } : undefined,
			body: body ? JSON.stringify(body) : undefined
		});
		if (response.ok && response.headers.get("Content-Type")?.includes("application/json"))
			return response.json();
		return response.bytes();
	}
	static get(e) { return this.request("GET", e); }
	static post(e, b) { return this.request("POST", e, b); }
	static put(e, b) { return this.request("PUT", e, b); }
	static delete(e) { return this.request("DELETE", e); }

};

// Ready-to-use calls to the rUFT API
// The server only allows localhost so it's secure by design.
// No, I won't install a Kubernetes server with Kerberos and IP logging just to
// INSTALL A DAMN CUSTOM ROM
export class Handy
{
	static async getAvailable() { return await Query.get("/api/available"); }
	static async getAvailableROMs() { return await Query.get("/api/available/rom"); }
	static async getAvailableRecoveries() { return await Query.get("/api/available/recovery"); }
	static async getAvailableRootModules() { return await Query.get("/api/available/stealth"); }
	static async getCodename() { return await Query.get("/api/devices"); }
}

export default {
	Query,
	Handy
}