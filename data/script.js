
function updateMeasuredValues() {
    fetch("/api/measured-values")
        .then(response => {
            if (!response.ok) {
                throw new Error("Network response was not ok");
            }
            return response.json();
        })
        .then(data => {
            // Update the measured values on your webpage using JavaScript
            document.getElementById("value1").textContent = data.value1.toFixed(2);
            document.getElementById("value2").textContent = data.value2.toFixed(2);
        })
        .catch(error => {
            console.error("Fetch error:", error);
        });
}


updateMeasuredValues();
setInterval(updateMeasuredValues, 1000); 




