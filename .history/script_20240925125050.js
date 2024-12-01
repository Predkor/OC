async function getProcessesList() {
    const ulr = "http://localhost:8080/processes";

    try{
        const response = await fetch(url);

        if (!response.ok) {
            
        }

    } catch (e) {
        console.log(`Error: ${e}`);
    }

}