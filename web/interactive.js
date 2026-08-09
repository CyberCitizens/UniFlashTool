
import { Handy, Query } from './api.js'

export 	const listsMap = {
			ROM : document.getElementById("romHolder"),
			Recovery : document.getElementById("recoveryHolder"),
			"Rooting tool" : document.getElementById("otherHolder"),
			"Rooting module" : document.getElementById("otherHolder"),
			"Application" : document.getElementById("otherHolder"), // future support of post-flash applications install
		};

// Creates a temporary dialog object and returns it.
export function Dialog(title)
{
	const [
		dialog,
		titleLabel,
		exit,
		exitIcon,
		displayArea,
	] = [
		document.createElement("dialog"),
		document.createElement("h2"),
		document.createElement("button"),
		document.createElement("img"),
		document.createElement("div"),
	];
	exitIcon.setAttribute("src", "/assets/icons/x.svg");
	titleLabel.textContent = title;
	titleLabel.classList.add('whole-line');
	dialog.legacyClose = dialog.close;
	dialog.classList.add("box");
	dialog.close = (returnValue = null) =>
	{
		dialog.legacyClose(returnValue);
		dialog.remove();
	}
	dialog.onclose += dialog.close();
	exit.addEventListener("click", e => 
	{
		dialog.close("dismiss");
	});
	// So we can only add elements in a non-destructive fashion.
	// If really needed, we can still use dialog.appendChild to use the method in a standard way.
	dialog.append = displayArea.append;
	exit.appendChild(exitIcon);
	dialog.appendChild(exit);
	dialog.appendChild(displayArea);
	dialog.appendChild(titleLabel);
	return dialog;
}

// Returns an HTML form that permits fast settings for a remote Archive.
// The window it runs in, defines what type of archive (Recovery, ROM, Module or app...) it is.
export function DefaultRemoteToolSettings(forcedType = null)
{
	// Tool type will be deduced depending on the context this runs in, to make the User more at ease and try to reduce friction.
	// Every information asked here is mandatory and cannot be deduced easily, or will be removed in the future.
	const [
		form,			// holder of other information
		name,			// tool name
		sourceType,		// type of source (ready-to-go archive, GitHub release, APK, image file...)
		targetDevice,	// nullable, defines the target device of this tool. In some cases, it's required (for architecture-gnostic tools, like image files or a ROM / Recovery image).
		source,			// where to find this tool from ? will be an URI / URL.
		version,		// nullable, required tool version.
		submit,			// self-explanatory if you have more than 5 IQ
	] = [
		document.createElement("form"),
		document.createElement("input"),
		document.createElement("select"),
		document.createElement("input"),
		document.createElement("input"),
		document.createElement("input"),
		document.createElement("input"),
	];
	const inputs = [
		name,
		source,
		sourceType,
		version,
		targetDevice,
	];

	const mandatory = [
		name,
		sourceType,
		source,
		submit,
	];

	form.classList.add("vertical");
	Handy.getCodename().then((result) => {
		console.log(result);
		targetDevice.value = result["codename"];
	}).catch((err) => {
		
	});

	name.placeholder = "Name";
	sourceType.placeholder = "Source Type (URL)";
	source.placeholder = "Source";
	version.placeholder = "Version";

	targetDevice.placeholder = "Target Device";
	targetDevice.disabled = "true";

	for(const i of mandatory)
		i.setAttribute("required", "true");
	for(const i of inputs)
		i.setAttribute("type", "text");
	
	const source_types =
	{
		"apk": "Android Package (APK)",
		"arc": "Remote Archive (.zip, .tar, .tar.gz...)",
		"img": "Remote Image (.img)",
		"git": "GitHub Repository (will pick the selected, or latest release)"
	}

	let source_types_options = []
	
	for(const source_type in source_types)
	{
		const option = document.createElement("option");
		option.textContent = source_types[source_type];
		option.value = source_type;
		source_types_options.push(option);
		sourceType.appendChild(option);
	}

	if(forcedType != null)
	{
		sourceType.selectedIndex = Object.getOwnPropertyNames(source_types).indexOf(forcedType);
		sourceType.disabled = "true";
	}

	form.append(...inputs);
	return form;
}

export function ShowAddTool()
{
	const dialog = Dialog("Add a new custom tool");
	document.body.prepend(dialog);
	dialog.append(DefaultRemoteToolSettings());
	dialog.showModal();
}

export function ShowAddROM()
{
	const dialog = Dialog("Add a new custom ROM");
	document.body.prepend(dialog);
	const holder = document.createElement("section");
	holder.style.display = "grid";
	holder.style.gridTemplateColumns = "1fr 1fr 1fr";
	for(let i of ["ROM", "DTBO", "Boot.img"])
	{
		const header = document.createElement('h3');
		header.textContent = i;
		holder.appendChild(header);
	}
	dialog.append(DefaultRemoteToolSettings());
	dialog.showModal();
}

export function ShowAddRecovery()
{
	const dialog = Dialog("Add a new custom Recovery Image");
	document.body.prepend(dialog);
	dialog.append(DefaultRemoteToolSettings("img"));
	dialog.showModal();
}

export async function GetAllSelectedTools()
{
	const DOMselect = document.querySelectorAll('.vertical-list[download]');
	let tools = [];
	for(let _domselect of DOMselect)
		tools.push({
			name: _domselect.getAttribute("toolName"),
			target_device: (await Handy.getCodename())["codename"]
		});
	return tools;
}

// Returns the associated <tr> in the downloads table
function GetOrCreateDownloadEntry(downloadId)
{
	const selector = `${downloadId}`.replaceAll(' ', '-').replaceAll('\'', '').replaceAll('\"', '')
	let entry = document.querySelector(`#${selector}`);
	if(entry != undefined)
		return {
			tr: entry,
			text: document.querySelector(`#${selector} > td`),
			progressbar: document.querySelector(`#${selector} > td > progress`),
			current: document.querySelector(`#${selector} > td:nth-child(3)`),
			max: document.querySelector(`#${selector} > td:nth-child(4)`),
		};
	entry = document.createElement("tr");
	const cell_text = document.createElement("td");
	const cell_current = document.createElement("td");
	const cell_max = document.createElement("td");
	const cell_progressBar = document.createElement("td");
	const progressBar = document.createElement("progress");
	entry.id = selector;

	cell_text.textContent = downloadId;
	cell_progressBar.appendChild(progressBar);
	entry.append(
		cell_text,
		cell_progressBar,
		cell_current,
		cell_max,
	);
	document.getElementById('dlTable').appendChild(entry);
	return {
		tr: entry,
		text: cell_text,
		progressbar: progressBar,
		current: cell_current,
		max: cell_max,
	};
}

export function DownloadEverything()
{
	const dialog = document.getElementById('dlDialog');
	/**
	 * @var { WebSocket } ws
	 */
	const ws = Handy.getDownloadSocket();
	ws.onmessage = e =>
	{
		dialog.style.display = "block";
		dialog.setAttribute('open', 'true');
		const data = JSON.parse(e.data);
		let tool = data["tool"];
		if(tool == undefined || tool == null)
			return;
		const kit = GetOrCreateDownloadEntry(tool["name"]);
		const expected = data["tool"]["expected_bytes"];
		const current = data["tool"]["current_bytes"];
		kit.progressbar.setAttribute("value", current);
		kit.progressbar.setAttribute("max", expected);
		kit.current.textContent = `${Math.round(parseFloat(current) / 1024 / 1024, 5)}MB`;
		kit.max.textContent = `${Math.round(parseFloat(expected) / 1024 / 1024, 5)}MB`;
		if(expected == current)
		{
			kit.tr.remove();
			GenerateToolLists(); // regenerates tool lists to guarantee a fresh state
		}
		if(document.querySelector('#dlTable').children.length == 1)
		{
			dialog.removeAttribute('open');
			dialog.style.display = "none";
		}
	}
	ws.send("start");
	// dialog.removeAttribute('open', 'true');
}

export async function GenerateToolLists()
{
	const toolbox = await Handy.getDownloadable();
	for(let category in toolbox)
	{
		listsMap[category].replaceChildren();
		toolbox[category].forEach(tool =>
		{
			const [ listElement, text, icons ] = [
				document.createElement('li'),
				document.createElement('p'),
				document.createElement('img'),
			];
			listElement.classList.add('vertical-list');
			listElement.setAttribute('toolName', tool);
			icons.classList.add('iconpack');
			icons.setAttribute('src', `/assets/pictures/${tool}.png`);
			icons.width = 32;
			icons.style.aspectRatio = "1/1";
			text.textContent = tool;
			listElement.appendChild(text);
			listElement.appendChild(icons);
			// When we be clicking on dat dumbass list element
			listElement.addEventListener('click', async e => {
				const _toolbox = await Handy.getAvailable();
				let available = false;
				let brandRange;
				if(_toolbox[category] != undefined && (brandRange = _toolbox[category][tool]) != undefined)
				{
					const brandAvailable = brandRange != undefined;
					for(let _tool of brandRange)
						if(_tool.target_device == (await Handy.getCodename())["codename"])
							available = true;
				}
				if(listElement.hasAttribute("download"))
				{
					// cancel download if queued, retry if failed
					// (ping API to retry too)
					switch (listElement.getAttribute("download")) {
						case "queued":
						case "available":
							listElement.removeAttribute("download");
							break;

						case "failed":
							listElement.setAttribute("download", "queued");
							// ping server to retry downloads here
							break;

						default: // it doesn't know what it is, f off
							console.error("The download can't really decide what it is, sorry. Sincerely, from your dear " + tool);
							break;
					}
				}
				else
					if(available)
						listElement.setAttribute("download", "available");
					else
						listElement.setAttribute("download", "queued");
			})
			listsMap[category].appendChild(listElement);
		});
	}
}

export default
{
	Dialog,
	DefaultRemoteToolSettings,
	DownloadEverything,
	ShowAddROM,
	ShowAddRecovery,
	ShowAddTool,
}