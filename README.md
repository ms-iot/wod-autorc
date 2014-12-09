The Self Driving RC Car
=======================

So, you want to make a self driving car. Ambitious. So are we. So we built one. Only, it was a little bit smaller than the real thing. 

You can build one too! In fact, we challenge you to do so. Here on this website, we'll tell you how we built ours, and hopefully that will give you a starting point for your own creation -- something faster, smarter, and cooler. 

When you've built it, share your design with us, along with a video of it working. The best projects will be rewarded for their awesomeness with cool Windows on Devices dev kits, shoutouts, and more!

But lets get right into it. This is how we built our car.

##The Supplies

We started with a car. Specifically a toy car. More specifically, a replica red (everyone knows red cars go faster) [Ford Mustang 1967 by Maisto Tech](http://www.amazon.com/Maisto-1967-Mustang-Colors-Styles/dp/B000FJEM14), complete with a racing stripe (and everyone knows cars with racing stripes are faster).

Next, we needed something to act as the brains of our car. Here at Microsoft, we've been working on [Windows on Devices](http://www.windowsondevices.com), a completely free, completely awesome, version of Windows that can run on devices (of which, we chose to use an [Intel Galileo Gen2](http://www.intel.com/content/www/us/en/do-it-yourself/galileo-maker-quark-board.html)).

Windows on Devices has all the goodness that you get when developing for Windows: Networking, File System support, plug and play USB, to name a few. But you get so much more: GPIO, PWM, SPI, and Arduino Compatibility. Familiar with Arduino? Great! You can write and copy your exisiting Sketch code on Windows, and it will just work!

Ok, we've got the car and the brains. We need a few more pieces:

####The eyes:

Since a car can't drive itself very well without knowing about the world around it, we decided to use 4 [Infared Proximity Sensors](https://www.sparkfun.com/products/242). These sensors deliver an analog output that is proportional to how close they are to an object, and have a range of 80cm. 

Secondly, most RC car toy DC motors don't have any kind of feedback, so it would be useful to know things like whether or not the car is moving, and how fast it's going. To accomplish these, we mounted a [Photoresistor](https://www.sparkfun.com/products/9088) in front of one of the forward wheels, which we painted half-white. We'll get into this a bit later.

####The motors:

The first thing we're going to do when we start building our self-driving car is to remove the existing internals that come with the RC Car. This leaves us without a motor driver to power the motors, so we suggest wiring up an L298N. We used an [L298N breakout board](http://www.amazon.com/SainSmart-Stepper-Controller-Mega2560-Duemilanove/dp/B00AJGM37I/ref=sr_1_3?ie=UTF8&qid=1417733989&sr=8-3&keywords=L298N).

##The Assembly

You've got what you need? Good. Let's put it together.

First, carefully take apart the RC Car (try not to lose all those screws, it makes putting the car back together again a bit difficult). When the top chassis is removed, carefully cut the leads to the circuit board inside the RC Car. Make note of which wires go to the battery positive and ground and which go to which motors. You may want to label them.

Next, connect the positive and negative leads from the battery to Vin and GND of the Motor Driver board respectively. Connect the rear motor to OUT3 and OUT4 of the motor driver, and the front motor to OUT1 and OUT2. 

After that, connect GND of the Galileo to GND of the Motor Driver board. 

Now, we want to set up the pins that will logically control our motors. Wire pins 5 and 6 from the Galileo to pins IN1 and IN2 of the Motor Driver board, and pins 11 and 12 of the Galileo to pins IN3 and IN4 of the Motor Driver board.

The Motor Driver board is powered by an L298N H-Bridge chip. By driving one input high and another low for each pair, you dictate the direction that current flows accross the respective motor. This, in turn, dictates the direction the motor turns in. 

We've chosen pins 5, 6, 11, and 12 as our driving pins because they are PWM enabled. This allows us to drive the motors at varying speeds. 

Next, we'll wire up our sensors. Each infared sensor has three leads, one yellow, one red, and one black. Wire the black wires to GND, the red to +5V.

Mount a sensor on the front of the car, angled 45 degrees left. Wire the yellow line from the sensor to A0 on the Galileo. 

Mount another sensor centered at the front, and wire that yellow wire to A1 on the Galileo.

Mount a third sensor angled 45 degrees right, sitting at the front of the car. Wire the yellow wire to A2 on the Galileo.

Finally, mount the fourth sensor centered at the rear of the car, and wire the yellow wire to A4.

These sensors will help us steer the car and avoid walls and obstacles. 

But we won't be able to avoid everything, since our setup doesn't have sensors to watch all 360 degrees around it. To fix this, we will mount a photoresistor to examine the state of one of the front wheels. First, you'll want to either paint half the wheel white, or tape a white piece of paper on half of the wheel. Now, mount the photoresistor on the bottom bumper so that it's facing the wheel. Secure the photoresistor so it doesn't move around if the car were to bump into stuff.

You'll want to connect one end of the photoresistor to both a 10kOhm resistor and A3 on the Galileo. The other end of the photoresistor should be wired directly to ground. Finally, wire the unconnected end of the 10kOhm resistor to +5V. 

If you're unclear about any of the instructions above, check out the Fritzing diagram below. 


Once you've got this all wired up, we're ready to start programming the brain behind our car!

##The Smarts (and the Software)

The software in our Autonomous RC Car will have a couple different components:

1. Front / Back Control
  This will be the most complicated part of our software. Here, we'll implement a simple PID controller (never heard of one before? You'll be able to read more about it down below!). The PID controller will drive the car, trying to find an optimal distance from the wall. At which point, we'll reverse the PID controller to use the rear sensor, and turn in random directions. 

2. Left / Right Control
  This will compare the values of the left and right sensors to determine when turning is neccessary. It'll be pretty straightforward, only consiting of a couple conditionals.

3. Speed and Stuck Detection
  This will use the photoresistor value to determine whether our car is stuck. We'll also talk about how you might use this reading to determine the speed of your car.

But before we talk about that, we need to cover some housekeeping.

###Housekeeping
In Visual Studio, create a new Galileo Sketch Project. First, we define constants for each of the sensors and their pins 

	//define input pins
	#define LEFT_SENSOR 2
	#define RIGHT_SENSOR 0
	#define FWD_SENSOR 1
	#define WHEEL_SENSOR 3
	#define REAR_SENSOR 4

	//define output pins
	#define OUT_RIGHT = 11;
	#define OUT_UP = 6;
	#define OUT_LEFT = 10;
	#define OUT_DOWN = 5;

We'll also define an enum to represent whether we are in forward or backwards driving mode, a buffer of ints to store the historical values of the wheel readings, and an array to store the id's of the sensors we're reading from and to place the values in. We'll also define a boolean where we keep track of whether or not the car is stuck.

	enum DRIVING_STATE { FORWARD, BACKWARD };
	DRIVING_STATE m_state;
	bool m_stopped = false;
	
	//statics in which to store readings from sensors
	const int SENSORS[] = { A0, A1, A2, A3, A4 };	
	int READINGS[] = { 0, 0, 0, 0, 0 };
	int WHEEL_READINGS[1000];

Next, lets take a look at what our sketch's loop() function will look like:

	//Loop Arduino function
	void loop()
	{
		//read in sensor values
		for (int i = 0; i < 5; i++)
		{
			READINGS[i] = analogRead(SENSORS[i]);
		}
	
		//perform median filtering on readings
		medianFilter();
		
		stuckDetection();
		drive();
	}

Our main logic loop is simple: read our sensors, run the values through a median filter to make sure we throw out noise and outliers, and then drive the car appropriately (depending on whether we are stuck or not).

Before moving on, we'll take a quick look at how the simple median filter works.

	void medianFilter()
	{
		static const size_t FRAME_SIZE = 20;
		static int data[6][FRAME_SIZE];
		static int sorted_data[6][FRAME_SIZE];
		static size_t data_size = 0;
	
		//shift data
		data_size = min(FRAME_SIZE, data_size + 1);
		for (int n = 0; n < 5; n++)
		{
			for (int i = data_size - 1; i > 0; i--) {
				data[n][i] = data[n][i - 1];
			}
		}
	
		//add in new value
		for (int n = 0; n < 5; n++)
		{
			data[n][0] = READINGS[n];
		}
	
		// Create a sorted array
		for (int n = 0; n < 5; n++)
		{
			memcpy(sorted_data[n], data[n], data_size * sizeof(int));
			std::qsort(sorted_data[n], data_size, sizeof(int), compare_ints);
		}
	
		// Get median
		for (int n = 0; n < 5; n++)
		{
			READINGS[n] = sorted_data[n][data_size / 2];
	 	}
	}

First, we define a frame size for each variable. Here, we use a frame size of 20 for each reading. That means the median filter will return the median value out of the last 20 readings.  It does this by sorting the list of values and returning the one in the middle. This is a way of averaging out readings, and should eliminate a lot of sensor noise and spikes, allowing for smoother control.

If you're having trouble wrapping your head around what the median filter does, take a look at the following two graphs. The first is a graph of a line ( y = x ) multiplied with a random value between 0.85 and 1.5. You can see a lot of spikes in the graph. The second graph is the same data, passed through a median filter with a frame size of 10. As you can see, the second graph still maintains the general response, but filters out many of the spikes (which if used as data to drive a controller would result in weird behavior).

![Raw vs Median Filtered Data](/images/raw-vs-filtered.png "Raw vs Median Filtered Data")

If you want to play around with this data, you can see the JavaScript snippet that we used to generate the example data, available in example-snippets/.

