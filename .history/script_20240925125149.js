async function getProcessesList() {
    const ulr = "http://localhost:8080/processes";

    try{
        const response = await fetch(url);

        if (!response.ok) {
            throw new Error(`Response status: ${response.status}`);
        }

        const json = await response.json();
        console.log(json);
    } catch (e) {
        console.log(`Error: ${e}`);
    }
}