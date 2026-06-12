const { Combination } = require('./build/Release/addon.node');

const comb = Combination.generate();
const sol = comb.solve()

console.log(`Target: ${comb.target}`)
console.log(`Numbers: ${comb.numbers.slice(0, 4).join(" ")}\t${comb.numbers[4]}\t${comb.numbers[5]}`)
console.log(`${sol}`)
