var chart = [];
var ws;
var dps = []

window.onload = function () {
	//initial value of dataPoints
	dps = [
		{label: "Atención", y: 50},
		{label: "Meditación", y: 80}
	];
	chart = new CanvasJS.Chart("mindPlot",{
		backgroundColor: "transparent",
		axisY:{
			gridColor: "rgba(255,255,255,.05)",
			tickColor: "rgba(255,255,255,.05)",
			labelFontColor: "#a2a2a2",
			maximum: 100,
			minimum: 0,
		},
		axisX:{
			labelFontColor: "#a2a2a2"
		},
		data:[{
			type: "column",
			bevelEnabled: true,
			dataPoints: dps
		}]
	});
	setupWs();
}

function setupWs() {
    var host = window.document.location.host.replace(/:.*/, '');
    ws = new WebSocket('ws://' + host + ':3000', 'client');

    ws.onmessage = function(event) {
        // console.log(event)
        var lecture = JSON.parse(event.data);
        console.log(lecture.attention);
				console.log(lecture.meditation);
        dps[0] = {
            label: "Atención",
            y: lecture.attention
        };
        dps[1] = {
            label: "Meditación",
            y: lecture.meditation
        };
        chart.render();
    };
}
