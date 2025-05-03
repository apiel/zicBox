# 40 Contribute to zicBox

🚧 **Help Wanted**: Whether you're a coder, designer, maker, or musician, your contributions are more than welcome!

The best way to get started is to reach out on the [Discord thread](https://discord.gg/65HTx7z9qg), let's have a quick chat and figure out how you can jump in.
Don't be shy, there are plenty of ways to get involved, no matter your background or experience.

[![](https://dcbadge.limes.pink/api/server/65HTx7z9qg)](https://discord.gg/65HTx7z9qg)

## 🎨 3D Enclosure Design

We're looking for makers and designers to help create a custom enclosure for Zic Pixel and other up coming designs. The idea is to house the PCB in a solid, visually appealing case. A 3D-printable enclosure is the most accessible option for DIY builders, but we're open to CNC, laser-cut, or other methods, as long as it's not too hard to reproduce.

Skills welcome:
- 3D modeling
- Knowledge of 3D printing and DIY fabrication
- Documentation of the process
- ...

## 🔌 PCB Design Improvements
The current PCB was designed in a self-taught, DIY style. It works, but the project is ready to evolve. The next steps include:

- Using SMD components and ordering pre-assembled boards
- Integrating components like DACs and I/O expanders directly onto the board
- Finally, on the last iteration, replace the CM4-NANO-A base, with including a CM4 connector, usb power, SD card read and usb host port.
- (Bonus) Exploring battery power options!

If you’re experienced in hardware design, or want to help refine and expand the current board, your input is highly appreciated.

Skills welcome:
- PCB design (currently working on EasyEDA)
- SMD component selection and layout
- Designing for manufacturing or open source shared project...

## 👨‍💻 Software Development
### 🧠 C++ Core
zicBox is primarily written in C++. The code is structured around reusable libraries, with plenty of room to expand:
- Create new audio modules
- Develop UI components
- Improve performance or interconnectivity between modules

Beginner to expert, every contribution is welcome, as long you know your way around C++.

> [!TIP]
>To simplify development, install a cross-compiler toolchain for ARM on your dev machine. In order to do this, let's use zicOs a custom OS create for zicBox, based on buildroot: https://github.com/apiel/zicOs
>
>Install alongside to your zicBox project:
>```
>- dev folder
>-- zicBox
>-- zicOs
>```

### 💻 TypeScript / JSX (Config & UI)
The configuration file is generated using TypeScript / JSX. Right now, the UI and the audio track configurations, are defined using some TS file that is then compiled to a JSON config file.

One long-term goal would be to build a web-based drag-and-drop UI for configuring and routing modules, making zicBox much easier to use.

Ways to help:
- Improve the existing TS-based config generator and polish the current UI
- Work on UI/UX for a future drag-and-drop editor

### 🎨 UX & Visual Design
Good design makes a huge difference, not only in how zicBox looks, but in how easy and enjoyable it is to use.

We’re looking for help with:

- UX Design: Simplifying and improving the user interface while keeping CPU usage minimal. Clean, low-overhead UI/UX patterns are key, thoughtful layout and flow.
- Visual Design: Logos, icons, UI assets, and visual elements for both the app and the GitHub/docs.
- Promotional Material: Graphics or layouts for sharing zicBox on social media, in videos, or presentations.

Skills welcome:
- UX thinking for embedded or constrained environments
- Design tools like Figma, Sketch, or even pen & paper
- Visual identity and branding for open source projects

## 📚 Documentation
Documentation often falls behind as development moves fast. We need help keeping things up-to-date and beginner-friendly.

Great contributions include:

- Updating documentation with recent changes
- Writing clear guides, diagrams, and module descriptions
- Improving the README and setup instructions
- Helping with onboarding and dev setup notes

## 🔍 Testing & Presets
Using zicBox and exploring its capabilities is a valuable form of contribution! You can help by testing features, reporting bugs, and creating preset examples or full projects.

Things to do:
- Create presets or musical setups
- Try new builds and report bugs or suggestions
- Share use cases and feedback

## 🎥 Videos & Demos
zicBox is best understood when seen in action. If you're into video content, your help is gold.

Ideas for video contributions:
- Walkthroughs and feature overviews
- Performance demos or live jams
- Setup tutorials and build guides
- Showcasing what zicBox can do!
